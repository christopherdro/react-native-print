
//  Created by Christopher Dro on 9/4/15.

#import "RNPrint.h"
#import <React/RCTConvert.h>
#import <React/RCTUtils.h>


@implementation RNPrint {
    RCTPromiseResolveBlock _resolve;
    RCTPromiseRejectBlock _reject;
    WKWebView *_webView;
}


- (dispatch_queue_t)methodQueue
{
    return dispatch_get_main_queue();
}

RCT_EXPORT_MODULE();

-(void)launchPrint:(NSData *) data
        resolver:(RCTPromiseResolveBlock)resolve
        rejecter:(RCTPromiseRejectBlock)reject {
    if(!_htmlString && ![UIPrintInteractionController canPrintData:data]) {
        reject(RCTErrorUnspecified, nil, RCTErrorWithMessage(@"Unable to print this filePath"));
        return;
    }
    
    UIPrintInteractionController *printInteractionController = [UIPrintInteractionController sharedPrintController];
    printInteractionController.delegate = self;
    
    // Create printing info
    UIPrintInfo *printInfo = [UIPrintInfo printInfo];
    
    printInfo.outputType = UIPrintInfoOutputGeneral;
    printInfo.jobName = _jobName;
    printInfo.duplex = UIPrintInfoDuplexLongEdge;
    printInfo.orientation = _isLandscape? UIPrintInfoOrientationLandscape: UIPrintInfoOrientationPortrait;
    
    printInteractionController.printInfo = printInfo;
    printInteractionController.showsPageRange = YES;
    
    if (_useWebView) {
        printInteractionController.printFormatter = _webView.viewPrintFormatter;
    } else if (_htmlString) {
        UIMarkupTextPrintFormatter *formatter = [[UIMarkupTextPrintFormatter alloc] initWithMarkupText:_htmlString];
        printInteractionController.printFormatter = formatter;
    } else {
        printInteractionController.printingItem = data;
    }
    
    // Completion handler
    void (^completionHandler)(UIPrintInteractionController *, BOOL, NSError *) =
    ^(UIPrintInteractionController *printController, BOOL completed, NSError *error) {
        if (!completed && error) {
            NSLog(@"Printing could not complete because of error: %@", error);
            reject(RCTErrorUnspecified, nil, RCTErrorWithMessage(error.description));
        } else {
            resolve(completed ? printInfo.jobName : nil);
        }
    };
    
    if (_pickedPrinter) {
      [_pickedPrinter contactPrinter:^(BOOL available) {
        if (available) {
          [printInteractionController printToPrinter:self->_pickedPrinter completionHandler:completionHandler];
        } else {
          reject(RCTErrorUnspecified, nil, RCTErrorWithMessage(@"Selected printer is unavailable"));
       }
      }];
    } else if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) { // iPad
        UIView *view = [[UIApplication sharedApplication] keyWindow].rootViewController.view;
        [printInteractionController presentFromRect:view.frame inView:view animated:YES completionHandler:completionHandler];
    } else { // iPhone
        [printInteractionController presentAnimated:YES completionHandler:completionHandler];
    }
}

-(void)loadPrinter {
    __block NSData *printData;
    BOOL isValidURL = NO;
    NSURL *candidateURL = [NSURL URLWithString: _filePath];
    if (candidateURL && candidateURL.scheme && candidateURL.host)
        isValidURL = YES;
    
    if (isValidURL) {
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *dataTask = [session dataTaskWithURL:[NSURL URLWithString:_filePath] completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self launchPrint:data resolver:_resolve rejecter:_reject];
            });
        }];
        [dataTask resume];
    } else {
        printData = [NSData dataWithContentsOfFile: _filePath];
        [self launchPrint:printData resolver:_resolve rejecter:_reject];
    }
}

-(void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    if (webView.isLoading) {
        return;
    }
    
    [self loadPrinter];
}

RCT_EXPORT_METHOD(print:(NSDictionary *)options
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    
    if (options[@"filePath"]){
        _filePath = [RCTConvert NSString:options[@"filePath"]];
    } else {
        _filePath = nil;
    }
    
    if (options[@"html"]){
        _htmlString = [RCTConvert NSString:options[@"html"]];
    } else {
        _htmlString = nil;
    }
    
    if (options[@"printerURL"]){
        _printerURL = [NSURL URLWithString:[RCTConvert NSString:options[@"printerURL"]]];
        _pickedPrinter = [UIPrinter printerWithURL:_printerURL];
    }
    
    if(options[@"isLandscape"]) {
        _isLandscape = [[RCTConvert NSNumber:options[@"isLandscape"]] boolValue];
    }

    if(options[@"jobName"]) {
        _jobName = [RCTConvert NSString:options[@"jobName"]];
    } else {
        _jobName = @"Document";
    }

    if (options[@"baseUrl"]) {
        _baseUrl = [NSURL URLWithString:options[@"baseUrl"]];
    }
    
    if (options[@"useWebView"]) {
        _useWebView = [[RCTConvert NSNumber:options[@"useWebView"]] boolValue];
    }
    
    if ((_filePath && _htmlString) || (_filePath == nil && _htmlString == nil)) {
        reject(RCTErrorUnspecified, nil, RCTErrorWithMessage(@"Must provide either `html` or `filePath`. Both are either missing or passed together"));
    }
    
    _resolve = resolve;
    _reject = reject;
    
    if (_useWebView) {
        _webView = [[WKWebView alloc] initWithFrame:self.bounds];
        _webView.navigationDelegate = self;
        [self addSubview:_webView];
        dispatch_async(dispatch_get_main_queue(), ^{
                [_webView loadHTMLString:_htmlString baseURL:_baseUrl];
        });
        
        return;
    }
    
    [self loadPrinter];
    
}

RCT_EXPORT_METHOD(selectPrinter:(NSDictionary *)options
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    
    UIPrinterPickerController *printPicker = [UIPrinterPickerController printerPickerControllerWithInitiallySelectedPrinter: _pickedPrinter];
    
    printPicker.delegate = self;
    
    void (^completionHandler)(UIPrinterPickerController *, BOOL, NSError *) =
    ^(UIPrinterPickerController *printerPicker, BOOL userDidSelect, NSError *error) {
        if (!userDidSelect && error) {
            NSLog(@"Printing could not complete because of error: %@", error);
            reject(RCTErrorUnspecified, nil, RCTErrorWithMessage(error.description));
        } else {
            [UIPrinterPickerController printerPickerControllerWithInitiallySelectedPrinter:printerPicker.selectedPrinter];
            if (userDidSelect) {
                _pickedPrinter = printerPicker.selectedPrinter;
                NSDictionary *printerDetails = @{
                                                 @"name" : _pickedPrinter.displayName,
                                                 @"url" : _pickedPrinter.URL.absoluteString,
                                                 };
                resolve(printerDetails);
            }
        }
    };
    
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) { // iPad
        UIView *view = [[UIApplication sharedApplication] keyWindow].rootViewController.view;
        CGFloat _x = 0;
        CGFloat _y = 0;
        if (options[@"x"]){
            _x = [RCTConvert CGFloat:options[@"x"]];
        }
        if (options[@"y"]){
            _y = [RCTConvert CGFloat:options[@"y"]];
        }
        [printPicker presentFromRect:CGRectMake(_x, _y, 0, 0) inView:view animated:YES completionHandler:completionHandler];
    } else { // iPhone
        [printPicker presentAnimated:YES completionHandler:completionHandler];
    }
}

#pragma mark - UIPrintInteractionControllerDelegate

-(UIViewController*)printInteractionControllerParentViewController:(UIPrintInteractionController*)printInteractionController  {
    UIViewController *result = [[[[UIApplication sharedApplication] delegate] window] rootViewController];
    while (result.presentedViewController) {
        result = result.presentedViewController;
    }
    return result;
}

-(void)printInteractionControllerWillDismissPrinterOptions:(UIPrintInteractionController*)printInteractionController {}

-(void)printInteractionControllerDidDismissPrinterOptions:(UIPrintInteractionController*)printInteractionController {}

-(void)printInteractionControllerWillPresentPrinterOptions:(UIPrintInteractionController*)printInteractionController {}

-(void)printInteractionControllerDidPresentPrinterOptions:(UIPrintInteractionController*)printInteractionController {}

-(void)printInteractionControllerWillStartJob:(UIPrintInteractionController*)printInteractionController {}

-(void)printInteractionControllerDidFinishJob:(UIPrintInteractionController*)printInteractionController {}

+(BOOL)requiresMainQueueSetup
{
  return YES;
}

@end
