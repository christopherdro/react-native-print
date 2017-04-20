package com.rnprint.RNPrint;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.content.Context;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.ParcelFileDescriptor;
import android.print.pdf.PrintedPdfDocument;
import android.print.PageRange;
import android.print.PrintAttributes;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentInfo;
import android.print.PrintManager;
import android.util.Log;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.Callback;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.UiThreadUtil;

import java.util.List;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;


/**
 * NativeModule that allows JS to open emails sending apps chooser.
 */
public class RNPrintModule extends ReactContextBaseJavaModule {

  ReactApplicationContext reactContext;

  public RNPrintModule(ReactApplicationContext reactContext) {
    super(reactContext);
    this.reactContext = reactContext;
  }

  @Override
  public String getName() {
    return "RNPrint";
  }

  WebView mWebView;

  @ReactMethod
  public void print(final String path, final Promise promise) {

    try {
      final String jobName = "Document";
      PrintManager printManager = (PrintManager) getCurrentActivity().getSystemService(Context.PRINT_SERVICE);

      PrintDocumentAdapter pda = new PrintDocumentAdapter() {

        @Override
        public void onWrite(PageRange[] pages, ParcelFileDescriptor destination, CancellationSignal cancellationSignal, WriteResultCallback callback){
          InputStream input = null;
          OutputStream output = null;

          try {
            input = new FileInputStream(path);
            output = new FileOutputStream(destination.getFileDescriptor());

            byte[] buf = new byte[1024];
            int bytesRead;

            while ((bytesRead = input.read(buf)) > 0) {
                 output.write(buf, 0, bytesRead);
            }

            callback.onWriteFinished(new PageRange[]{PageRange.ALL_PAGES});

            try {
                input.close();
                output.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

          } catch (FileNotFoundException ee){
            // Catch exception
            Log.e("RNPrint", "file not found");
          } catch (Exception e) {
            // Catch exception
            Log.e("RNPrint", "other exception");
          }
        }

        @Override
        public void onLayout(PrintAttributes oldAttributes, PrintAttributes newAttributes, CancellationSignal cancellationSignal, LayoutResultCallback callback, Bundle extras){

          if (cancellationSignal.isCanceled()) {
            callback.onLayoutCancelled();
            return;
          }

          PrintDocumentInfo pdi = new PrintDocumentInfo.Builder(jobName).setContentType(PrintDocumentInfo.CONTENT_TYPE_DOCUMENT).build();

          callback.onLayoutFinished(pdi, true);
        }
      };


      printManager.print(jobName, pda, null);
      promise.resolve(jobName);

    } catch (Exception e) {
      promise.reject("print_error", e);
    }

  }

  @ReactMethod
  public void printhtml(final String html, final Promise promise) {

    final String jobName = "Document";

    try {
      UiThreadUtil.runOnUiThread(new Runnable() {
          @Override
          public void run() {
              // Create a WebView object specifically for printing
              WebView webView = new WebView(reactContext);
              webView.setWebViewClient(new WebViewClient() {
                public boolean shouldOverrideUrlLoading(WebView view, String url) {
                    return false;
                }

                @Override
                public void onPageFinished(WebView view, String url) {
                    // Get the print manager.
                    PrintManager printManager = (PrintManager) getCurrentActivity().getSystemService(
                            Context.PRINT_SERVICE);
                    // Create a wrapper PrintDocumentAdapter to clean up when done.
                    PrintDocumentAdapter adapter = new PrintDocumentAdapter() {
                        private final PrintDocumentAdapter mWrappedInstance =
                                mWebView.createPrintDocumentAdapter();
                        @Override
                        public void onStart() {
                            mWrappedInstance.onStart();
                        }
                        @Override
                        public void onLayout(PrintAttributes oldAttributes, PrintAttributes newAttributes,
                                CancellationSignal cancellationSignal, LayoutResultCallback callback,
                                Bundle extras) {
                            mWrappedInstance.onLayout(oldAttributes, newAttributes, cancellationSignal,
                                    callback, extras);
                        }
                        @Override
                        public void onWrite(PageRange[] pages, ParcelFileDescriptor destination,
                                CancellationSignal cancellationSignal, WriteResultCallback callback) {
                            mWrappedInstance.onWrite(pages, destination, cancellationSignal, callback);
                        }
                        @Override
                        public void onFinish() {
                            mWrappedInstance.onFinish();
                        }
                    };
                    // Pass in the ViewView's document adapter.
                    printManager.print(jobName, adapter, null);
                    mWebView = null;
                    promise.resolve(jobName);
                }
              });

              webView.loadDataWithBaseURL(null, html, "text/HTML", "UTF-8", null);

              // Keep a reference to WebView object until you pass the PrintDocumentAdapter
              // to the PrintManager
              mWebView = webView;
          }
      });
    } catch (Exception e) {
      promise.reject("print_error", e);
    }
  }
}
