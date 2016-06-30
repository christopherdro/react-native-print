# react-native-print

Print documents using React Native.

### Add it to your project

1. Run `npm install react-native-print --save`
2. Open your project in XCode, right click on [Libraries](http://url.brentvatne.ca/jQp8) and select [Add Files to "Your Project Name](http://url.brentvatne.ca/1gqUD).
3. Choose the file `node_modules/react-native-print/RNPrint.xcodeproj`
4. Go to `Project Manager` tab and click on your project's name. Select the name of the target and click on `Build Phases`
5. Add `libRNPrint.a` to `Link Binary With Libraries`
   [(Screenshot)](http://url.brentvatne.ca/17Xfe).

## Usage
```javascript

import React from 'react';
import {
  AlertIOS,
  AppRegistry,
  StyleSheet,
  Text,
  TouchableHighlight,
  View,
} from 'react-native';
import {RNPrint} from 'NativeModules';

var Example = React.createClass({

  printDocument(filePath) {
    RNPrint.print(filePath).then((jobName) => {
      console.log(`Printing ${jobName} complete!`);
    });;
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
