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
import com.rnprint.RNPrint.RNPrint;

// include package
new MainReactPackage(),
new RNPrint(),
```


## Usage
```javascript

import React from 'react';
import {
  Platform,
  Text,
  TouchableHighlight,
  View,
} from 'react-native';
import RNPrint from 'react-native-print';

var Example = React.createClass({

  printDocument(filePath) {
    RNPrint.print(filePath).then((jobName) => {
      console.log(`Printing ${jobName} complete!`);
    });;
  },

  printHTML(htmlString) {
    // only available on android
    if (Platform.OS === 'android'){
      RNPrint.printhtml(htmlString).then((jobName) => {
        console.log(`Printing ${jobName} complete!`);
      });
    }
  },

  render() {
    <View>
      <TouchableHighlight onPress={this.printPDF(filePath)}>
        <Text>Print Document</Text>
      </TouchableHighlight>
    </View>
  }
});
```

## Example
The example project included demonstrates how you use `react-native-html-to-pdf` to create a PDF from a html string then prompt the print dialog.
