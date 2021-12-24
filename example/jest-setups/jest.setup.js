import {windowsAppDriverCapabilities} from 'selenium-appium';

switch (platform) {
  case 'windows':
    const webViewWindowsAppId = 'RNPrintExample_dxa658cp5a26j!App';
    module.exports = {
      capabilites: windowsAppDriverCapabilities(webViewWindowsAppId),
    };
    break;
  default:
    throw 'Unknown platform: ' + platform;
}
