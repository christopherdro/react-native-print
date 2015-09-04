# react-native-print

Print documents using React Native.

### Add it to your project

1. Run `npm install react-native-print --save`
2. Open your project in XCode, right click on [Libraries](http://url.brentvatne.ca/jQp8) and select [Add Files to "Your Project Name](http://url.brentvatne.ca/1gqUD).
3. Add `libRNPrint.a` to `Build Phases -> Link Binary With Libraries`
   [(Screenshot)](http://url.brentvatne.ca/17Xfe).

## Usage
```javascript

var React = require('react-native');

var {
  AlertIOS,
  AppRegistry,
  NativeModules: {
    RNPrint,
  }
  StyleSheet,
  Text,
  TouchableHighlight,
  View,
} = React;

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
