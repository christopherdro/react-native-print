#include "pch.h"
#include "RNPrint.h"

namespace winrt::RNPrint
{
  // Searches for the RNPrintCanvas element in the visual tree.
  Canvas RNPrint::searchForPrintCanvas(xaml::DependencyObject startNode)
  {
    const int count = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(startNode);
    Canvas result = nullptr;
    for (int i = 0; i < count; i++)
    {
      xaml::DependencyObject current = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(startNode, i);
      Canvas temp = current.try_as<Canvas>();
      if (temp)
      {
        std::string tempName = winrt::to_string(temp.Name());
        if (tempName == RNPrintCanvasName)
        {
          return temp;
        }
      } else
      {
        result = searchForPrintCanvas(current);
        if (result)
        {
          break;
        }
      }
    }
    return result;
  }

  // cleans up local variables and XAML elements.
  void RNPrint::cleanUp()
  {
    reactContext.UIDispatcher().Post([=]()
      {
        auto printMan = PrintManager::GetForCurrentView();
        {
          std::lock_guard<std::mutex> guard(printSync);
          servingPrintRequest = false;
          if (printToken)
          {
            printMan.PrintTaskRequested(printToken);
          }
          printToken = {};
          pageCollection.clear();
          if (printCanvas)
          {
            printCanvas.Children().Clear();
          }
        }
      }
    );
  }

  // Generates a XAML page asynchronously.
  concurrency::task<std::optional<xaml::UIElement>> RNPrint::GeneratePageAsync(
    PdfDocument pdfDocument,
    int pageNumber,
    PrintPageDescription pageDescription)
  {
    Image pageImage;
    PdfPage pdfPage = pdfDocument.GetPage(pageNumber - 1);
    BitmapImage pageSource;
    pageImage.Source(pageSource);
    InMemoryRandomAccessStream pageStream;
    Canvas page;
    page.Width(pageDescription.PageSize.Width);
    page.Height(pageDescription.PageSize.Height);

    const float leftMargin = pageDescription.ImageableRect.X;
    const float topMargin = pageDescription.ImageableRect.Y;
    const float printableWidth = pageDescription.ImageableRect.Width;
    const float printableHeight = pageDescription.ImageableRect.Height;

    Canvas printableAreaCanvas;
    printableAreaCanvas.Width(printableWidth);
    printableAreaCanvas.Height(printableWidth);

    auto pdfContentDimensions = pdfPage.Dimensions().MediaBox();
    float pdfContentWidth = pdfContentDimensions.Width;
    float pdfContentHeight = pdfContentDimensions.Height;

    //Scale to Fit logic
    const float scale = min(printableHeight / pdfContentHeight, printableWidth / pdfContentWidth);
    pdfContentWidth *= scale;
    pdfContentHeight *= scale;
    Canvas::SetLeft(printableAreaCanvas, (double)leftMargin + (printableWidth - pdfContentWidth) / 2);
    Canvas::SetTop(printableAreaCanvas, (double)topMargin + (printableHeight - pdfContentHeight) / 2);

    auto renderOptions = PdfPageRenderOptions();
    renderOptions.SourceRect(pdfContentDimensions);
    renderOptions.DestinationWidth(pdfContentWidth);
    renderOptions.DestinationHeight(pdfContentHeight);

    printableAreaCanvas.Children().Append(pageImage);
    page.Children().Append(printableAreaCanvas);

    // Can't block or co_await on this Delegate.
    auto renderToStreamTask = pdfPage.RenderToStreamAsync(pageStream, renderOptions);
    return concurrency::create_task([=]
      {
        renderToStreamTask.get();
        reactContext.UIDispatcher().Post([=]()
          {
            // Needs to be run on UI thread.
            pageSource.SetSource(pageStream);
          });
      }).then([=]() -> std::optional<xaml::UIElement>
        {
          return page;
        });
  }

  // Asynchronously loads the PDF document, registers print callbacks and fires the print UI.
  IAsyncAction RNPrint::PrintAsyncHelper(
    RNPrintOptions options,
    ReactPromise<JSValue> promise) noexcept
  {
    auto capturedPromise = promise;
    auto capturedOptions = options;


    if ((!capturedOptions.html && !capturedOptions.filePath) || (capturedOptions.html && capturedOptions.filePath))
    {
      cleanUp();
      capturedPromise.Reject("Must provide either 'html' or 'filePath'. Both are either missing or passed together.");
      co_return;
    }

    if (capturedOptions.html)
    {
      cleanUp();
      capturedPromise.Reject("Printing HTML not supported");
      co_return;
    } else
    {
      // Pdf print instructions.
      winrt::Windows::Foundation::Uri uri(winrt::to_hstring(*capturedOptions.filePath));

      bool isValidURL = !uri.Host().empty() && (winrt::to_string(uri.SchemeName()) == "http" || winrt::to_string(uri.SchemeName()) == "https");
      PdfDocument pdfDocument = nullptr;

      if (isValidURL)
      {
        // Should be a valid URL.
        auto httpClient = HttpClient();
        auto httpResponseMessage = co_await httpClient.GetAsync(uri);
        auto memoryStream = InMemoryRandomAccessStream();
        co_await httpResponseMessage.Content().WriteToStreamAsync(memoryStream.GetOutputStreamAt(0));
        pdfDocument = co_await PdfDocument::LoadFromStreamAsync(memoryStream);
      } else
      {
        // Not a valid URL, try to open as a file.
        StorageFile pdfFile = nullptr;
        pdfFile = co_await StorageFile::GetFileFromPathAsync(winrt::to_hstring(*capturedOptions.filePath));
        pdfDocument = co_await PdfDocument::LoadFromFileAsync(pdfFile);
      }
      if (pdfDocument == nullptr)
      {
        cleanUp();
        capturedPromise.Reject("Couldn't open the PDF file.");
        co_return;
      } else
      {
        auto printMan = PrintManager::GetForCurrentView();
        printDoc = PrintDocument();
        printDocSource = printDoc.DocumentSource();

        printDoc.Paginate(
          [=](auto const& sender, PaginateEventArgs const& args)
          {
            {
              // Clears the XAML pages.
              std::lock_guard<std::mutex> guard(printSync);
              pageCollection.clear();
              printCanvas.Children().Clear();
            }
            auto printTaskOptions = args.PrintTaskOptions();
            printPageDescr = printTaskOptions.GetPageDescription(0);
            numberOfPages = pdfDocument.PageCount();
            sender.as<PrintDocument>().SetPreviewPageCount(numberOfPages, PreviewPageCountType::Final);
          }
        );

        printDoc.GetPreviewPage(
          [=](auto const& sender, GetPreviewPageEventArgs const& args) -> void
          {
            auto printDocArg = sender.as<PrintDocument>();
            const int pageNumber = args.PageNumber();
            GeneratePageAsync(pdfDocument, pageNumber, printPageDescr).then([=](std::optional<xaml::UIElement> generatedPage)
              {
                reactContext.UIDispatcher().Post([=]()
                  {
                    {
                      std::lock_guard<std::mutex> guard(printSync);
                      printCanvas.Children().Append(*generatedPage);
                      printCanvas.InvalidateMeasure();
                      printCanvas.UpdateLayout();
                    }
                    printDocArg.SetPreviewPage(pageNumber, *generatedPage);
                  });
              }
            );
          }
        );

        printDoc.AddPages(
          [=](auto const& sender, AddPagesEventArgs const& args)
          {
            PrintDocument printDoc = sender.as<PrintDocument>();
            std::vector<concurrency::task<void>> createPageTasks;

            // Generate tasks for each page that needs to be sent to the printer.
            for (int i = 0; i < numberOfPages; i++)
            {
              createPageTasks.push_back(GeneratePageAsync(pdfDocument, i + 1, printPageDescr).then([=](std::optional<xaml::UIElement> generatedPage)
                {
                  pageCollection[i] = *generatedPage;
                }
              ));
            }

            // After all pages have been generated, submit them for printing.
            concurrency::when_all(createPageTasks.begin(), createPageTasks.end()).then(
              [=]()
              {
                reactContext.UIDispatcher().Post([=]()
                  {
                    std::for_each(pageCollection.begin(), pageCollection.end(),
                      [=](auto keyValue)
                      {

                        printDoc.AddPage(*(keyValue.second));
                      }
                    );
                    printDoc.AddPagesComplete();
                  }
                );
              }
            );

          });

        printToken = printMan.PrintTaskRequested(
          [=](PrintManager const& sender, PrintTaskRequestedEventArgs const& args)
          {
            auto printTask = args.Request().CreatePrintTask(winrt::to_hstring(capturedOptions.jobName),
              [=](PrintTaskSourceRequestedArgs const& args)
              {
                args.SetSource(printDocSource);
              }
            );
            if (options.isLandscape)
            {
              printTask.Options().Orientation(PrintOrientation::Landscape);
            } else
            {
              printTask.Options().Orientation(PrintOrientation::Portrait);
            }

            printTask.Completed([=](PrintTask const& sender, PrintTaskCompletedEventArgs const& args)
              {
                cleanUp();
                switch (args.Completion())
                {
                case PrintTaskCompletion::Failed:
                  capturedPromise.Reject("Failed to print.");
                  break;
                case PrintTaskCompletion::Abandoned:
                  capturedPromise.Reject("Printing Abandoned");
                  break;
                default:
                  capturedPromise.Resolve(options.jobName);
                  break;
                }
              }
            );
          }
        );

        co_await PrintManager::ShowPrintUIAsync();
      }
    }
  }

  void RNPrint::Print(JSValueObject&& options, ReactPromise<JSValue> promise) noexcept
  {
    bool ok_to_go_ahead = true;
    {
      // Only support one print request at a time.
      std::lock_guard<std::mutex> guard(printSync);
      if (servingPrintRequest)
      {
        ok_to_go_ahead = false;
      } else
      {
        servingPrintRequest = true;
      }
    }
    if (!ok_to_go_ahead)
    {
      promise.Reject("Another print request is already being served.");
      return;
    }

    RNPrintOptions printOptions;

    if (options.find("html") != options.end())
    {
      printOptions.html = options["html"].AsString();
    }

    std::optional<std::string> filePath = std::nullopt;
    if (options.find("filePath") != options.end())
    {
      printOptions.filePath = options["filePath"].AsString();
    }

    printOptions.isLandscape = (options.find("isLandscape") != options.end() ? options["isLandscape"].AsBoolean() : false);
    printOptions.jobName = (options.find("jobName") != options.end() ? options["jobName"].AsString() : defaultJobName);
    reactContext.UIDispatcher().Post([=]()
      {
        xaml::FrameworkElement root{ nullptr };

        auto window = xaml::Window::Current();

        if (window != nullptr)
        {
          root = window.Content().try_as<xaml::FrameworkElement>();
        } else
        {
          if (auto xamlRoot = React::XamlUIService::GetXamlRoot(reactContext.Properties().Handle()))
          {
            root = xamlRoot.Content().try_as<xaml::FrameworkElement>();
          }
        }

        if (!root)
        {
          cleanUp();
          promise.Reject("A valid XAML root was not found.");
          return;
        }

        printCanvas = searchForPrintCanvas(root);

        if (!printCanvas)
        {
          cleanUp();
          promise.Reject("The XAML Canvas named \"RNPrintCanvas\" was not found.");
          return;
        }

        auto asyncOp = PrintAsyncHelper(printOptions, promise);
        asyncOp.Completed([=](auto action, auto status)
          {
            // Here we handle any unhandled exceptions thrown during the
            // asynchronous call by rejecting the promise with the error code
            if (status == winrt::Windows::Foundation::AsyncStatus::Error)
            {
              cleanUp();
              std::stringstream errorCode;
              errorCode << "0x" << std::hex << action.ErrorCode() << std::dec << std::endl;

              auto error = winrt::Microsoft::ReactNative::ReactError();
              error.Message = "HRESULT " + errorCode.str() + ": " + std::system_category().message(action.ErrorCode());
              promise.Reject(error);
            }
          });
      });
  }
}
