# react-native-print

Print documents using React Native.

## Installation

Run `npm install react-native-print --save`

## Add it to your project

### Automatic

Run `react-native link`

### Manual

#### iOS
1. Open your project in XCode, right click on [Libraries](http://url.brentvatne.ca/jQp8) and select [Add Files to "Your Project Name](http://url.brentvatne.ca/1gqUD).
2. Choose the file `node_modules/react-native-print/RNPrint.xcodeproj`
3. Go to `Project Manager` tab and click on your project's name. Select the name of the target and click on `Build Phases`
4. Add `libRNPrint.a` to `Link Binary With Libraries`
   [(Screenshot)](http://url.brentvatne.ca/17Xfe).

#### Android
- Edit `android/settings.gradle` to included

```java
include ':react-native-print'
project(':react-native-print').projectDir = new File(rootProject.projectDir,'../node_modules/react-native-print/android')
```

- Edit `android/app/build.gradle` file to include

```java
dependencies {
  ....
  compile project(':react-native-print')

}
```

- Edit `MainApplication.java` to include

```java
// import the package
import com.christopherdro.RNPrint.RNPrintPackage;

// include package
new MainReactPackage(),
new RNPrintPackage(),
```


## Usage
```javascript
/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 * @flow
 */

import React, { Component } from 'react';
import {
  AppRegistry,
  Button,
  StyleSheet,
  NativeModules,
  Platform,
  Text,
  View
} from 'react-native';


import RNHTMLtoPDF from 'react-native-html-to-pdf';
import RNPrint from 'react-native-print';

export default class RNPrintExample extends Component {
  state = {
    selectedPrinter: null
  }

  // @NOTE iOS Only
  selectPrinter = async () => {
    const selectedPrinter = await RNPrint.selectPrinter({ x: 100, y: 100 })
    this.setState({ selectedPrinter })
  }

  // @NOTE iOS Only
  silentPrint = async () => {
    if (!this.state.selectedPrinter) {
      alert('Must Select Printer First')
    }

    const jobName = await RNPrint.print({
      printerURL: this.state.selectedPrinter.url,
      html: '<h1>Silent Print</h1>'
    })

  }

  async printHTML() {
    await RNPrint.print({
      html: '<h1>Heading 1</h1><h2>Heading 2</h2><h3>Heading 3</h3>'
    })
  }

  async printPDF() {
    const results = await RNHTMLtoPDF.convert({
      html: '<h1>Custom converted PDF Document</h1>',
      fileName: 'test',
      base64: true,
    })

    await RNPrint.print({ filePath: results.filePath })
  }

  async printRemotePDF() {
    await RNPrint.print({ filePath: 'https://graduateland.com/api/v2/users/jesper/cv' })
  }

  customOptions = () => {
    return (
      <View>
        {this.state.selectedPrinter &&
          <View>
            <Text>{`Selected Printer Name: ${this.state.selectedPrinter.name}`}</Text>
            <Text>{`Selected Printer URI: ${this.state.selectedPrinter.url}`}</Text>
          </View>
        }
      <Button onPress={this.selectPrinter} title="Select Printer" />
      <Button onPress={this.silentPrint} title="Silent Print" />
    </View>

    )
  }

  render() {
    return (
      <View style={styles.container}>
        {Platform.OS === 'ios' && this.customOptions()}
        <Button onPress={this.printHTML} title="Print HTML" />
        <Button onPress={this.printPDF} title="Print PDF" />
        <Button onPress={this.printRemotePDF} title="Print Remote PDF" />
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
});

```

### Methods

## print(options: Object)
| Param | Type | Note |
|---|---|---|
| `html` | `string` |  HTML string to print
| `fileName` | `string` | Custom Filename excluding .pdf extension or remote file url NOTE: iOS only supports https protocols
| `printerURL` | `string` | **iOS Only:** URL returned from `selectPrinterMethod()`
| `isLandscape` | `bool` | Landscape print; default value is false
| `jobName` | `string` | **Android Only:** Name of printing job; default value is "Document"


## selectPrinter(options: Object)
| Param | Type | Note |
|---|---|---|
| `x` | `string` | **iPad Only:** The x position of the popup dialog
| `y` | `string` | **iPad Only:** The y position of the popup dialog
