package com.christopherdro.RNPrint;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.ParcelFileDescriptor;
import android.print.PageRange;
import android.print.PrintAttributes;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentInfo;
import android.print.PrintManager;
import android.webkit.URLUtil;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.annotation.RequiresApi;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.ReadableMap;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.UiThreadUtil;
import com.facebook.react.modules.network.CookieJarContainer;
import com.facebook.react.modules.network.ForwardingCookieHandler;
import com.facebook.react.modules.network.OkHttpClientProvider;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import okhttp3.JavaNetCookieJar;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * NativeModule that allows JS to open emails sending apps chooser.
 */
public class RNPrintModule extends ReactContextBaseJavaModule {

    ReactApplicationContext reactContext;
    final String defaultJobName = "Document";


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
    public void print(final ReadableMap options, final Promise promise) {

        final String html = options.hasKey("html") ? options.getString("html") : null;
        final String filePath = options.hasKey("filePath") ? options.getString("filePath") : null;
        final boolean isLandscape = options.hasKey("isLandscape") ? options.getBoolean("isLandscape") : false;
        final String jobName = options.hasKey("jobName") ? options.getString("jobName") : defaultJobName;

        if ((html == null && filePath == null) || (html != null && filePath != null)) {
            promise.reject(getName(), "Must provide either `html` or `filePath`.  Both are either missing or passed together");
            return;
        }

        if (html != null) {
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
                                        promise.resolve(jobName);
                                    }
                                };
                                // Pass in the ViewView's document adapter.
                                printManager.print(jobName, adapter, null);
                                mWebView = null;
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
        } else {
            try {

                PrintManager printManager = (PrintManager) getCurrentActivity().getSystemService(Context.PRINT_SERVICE);
                PrintDocumentAdapter pda = new PrintDocumentAdapter() {

                    @Override
                    public void onWrite(PageRange[] pages, final ParcelFileDescriptor destination, CancellationSignal cancellationSignal, final WriteResultCallback callback){
                        try {
                            boolean isUrl = URLUtil.isValidUrl(filePath);

                            if (isUrl) {
                                new Thread(new Runnable() {
                                    public void run() {
                                        CookieJarContainer cookieJarContainer = null;

                                        try {
                                            OkHttpClient client = OkHttpClientProvider.createClient();
                                            ForwardingCookieHandler cookieHandler = new ForwardingCookieHandler(reactContext);
                                            cookieJarContainer = (CookieJarContainer) client.cookieJar();
                                            cookieJarContainer.setCookieJar(new JavaNetCookieJar(cookieHandler));

                                            Request.Builder requestBuilder = new Request.Builder().url(filePath);
                                            Response res = client.newCall(requestBuilder.build()).execute();

                                            loadAndClose(destination, callback, res.body().byteStream());

                                            res.close();
                                        } catch (Exception e) {
                                            e.printStackTrace();
                                        } finally {
                                            if (cookieJarContainer != null) {
                                                cookieJarContainer.removeCookieJar();
                                            }
                                        }
                                    }
                                }).start();
                            } else {
                                InputStream input = new FileInputStream(filePath);
                                loadAndClose(destination, callback, input);
                            }

                        } catch (FileNotFoundException ee){
                            promise.reject(getName(), "File not found");
                        } catch (Exception e) {
                            // Catch exception
                            promise.reject(getName(), e);
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

                    @Override
                    public void onFinish() {
                        promise.resolve(jobName);
                    }
                };

                PrintAttributes printAttributes = new PrintAttributes.Builder()
                        .setMediaSize(isLandscape?PrintAttributes.MediaSize.UNKNOWN_LANDSCAPE:PrintAttributes.MediaSize.UNKNOWN_PORTRAIT)
                        .build();
                printManager.print(jobName, pda, printAttributes);

            } catch (Exception e) {
                promise.reject(getName(), e);
            }
        }
    }


    private void loadAndClose(ParcelFileDescriptor destination, PrintDocumentAdapter.WriteResultCallback callback, InputStream input) throws IOException {
        OutputStream output = null;
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
    }

}
