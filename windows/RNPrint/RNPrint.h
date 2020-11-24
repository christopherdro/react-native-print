#pragma once

#include "pch.h"
#include "NativeModules.h"

#include <functional>
#include <sstream>
#include <mutex>
#include <string_view>

#include "JSValue.h"

using namespace winrt::Microsoft::ReactNative;
using namespace winrt::Windows::Data::Pdf;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Web::Http;
using namespace winrt::Windows::Graphics::Printing;
using namespace winrt::Windows::Graphics::Printing::OptionDetails;
using namespace winrt::Windows::UI::Xaml::Printing;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media::Imaging;
using namespace winrt::Windows::UI::Xaml::Media;

namespace winrt::RNPrint
{
  // Represent the options object passed to print()
  struct RNPrintOptions
  {
    std::optional<std::string> html = std::nullopt;
    std::optional<std::string> filePath = std::nullopt;
    bool isLandscape = false;
    std::string jobName;
  };

  REACT_MODULE(RNPrint);
  struct RNPrint
  {
    inline static constexpr std::string_view Name = "RNPrint";

    // Default name for the print job.
    inline static constexpr std::string_view defaultJobName = "Document";

    // Name for the XAML Canvas to add preview folders to.
    inline static constexpr std::string_view RNPrintCanvasName = "RNPrintCanvas";

    // These are needed between print PDF callbacks, so we place them in the struct.
    ReactContext reactContext = nullptr;
    Canvas printCanvas = nullptr; // holds reference to RNPrintCanvas in visual tree
    PrintPageDescription printPageDescr;
    PrintDocument printDoc = nullptr;
    IPrintDocumentSource printDocSource;
    int numberOfPages = 0;
    std::map<int, std::optional<xaml::UIElement>> pageCollection; // references to the xaml pages
    std::mutex printSync; // prevent race conditions when manipulating pageCollection or checking a print request is being processed
    bool servingPrintRequest = false;

    // Event token for print registratio.
    winrt::event_token printToken;

    REACT_INIT(RNPrint_Init);
    void RNPrint_Init(ReactContext const& context) noexcept
    {
      reactContext = context;
    }

    // Searches for the RNPrintCanvas element in the visual tree.
    Canvas searchForPrintCanvas(xaml::DependencyObject startNode);

    // cleans up local variables and XAML elements.
    void cleanUp();

    // Generates a XAML page asynchronously.
    concurrency::task<std::optional<xaml::UIElement>> GeneratePageAsync(
      PdfDocument pdfDocument,
      int pageNumber,
      PrintPageDescription pageDescription);

    // Asynchronously loads the PDF document, registers print callbacks and fires the print UI.
    IAsyncAction PrintAsyncHelper(
      RNPrintOptions options,
      ReactPromise<JSValue> promise) noexcept;

    REACT_METHOD(Print, L"print");
    void Print(JSValueObject&& options, ReactPromise<JSValue> promise) noexcept;

  };
}
