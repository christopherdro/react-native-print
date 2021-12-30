
//  Created by Christopher on 9/4/15.

#import <UIKit/UIKit.h>
#import <React/RCTView.h>
#import <React/RCTBridgeModule.h>

@interface RNPrint : RCTView <RCTBridgeModule, UIPrintInteractionControllerDelegate, UIPrinterPickerControllerDelegate>
@property UIPrinter *pickedPrinter;
@property NSString *filePath;
@property NSString *htmlString;
@property NSString *jobName;
@property NSURL *printerURL;
@property NSURL *baseUrl;
@property (nonatomic, assign) BOOL isLandscape;
@property (nonatomic, assign) BOOL useWebView;
@end
