import {driver, By2} from 'selenium-appium';
import {until} from 'selenium-webdriver';

const setup = require('../jest-setups/jest.setup');
jest.setTimeout(60000);

beforeAll(() => {
  return driver.startWithCapabilities(setup.capabilites);
});

afterAll(() => {
  return driver.quit();
});

describe('Test App', () => {
  test('Opens Print UI', async () => {
    await driver.wait(until.elementLocated(By2.nativeName('Print Remote PDF')));
    (await driver.findElement(By2.nativeName('Print Remote PDF'))).click();
    await driver.wait(until.elementLocated(By2.nativeName('Document - Print')));
    (await driver.findElement(By2.nativeName('Cancel'))).click();
    // Wait for print popup to close.
    await driver.sleep(2000);
  });
});
