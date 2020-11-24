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
    const selectedPrinter = await RNPrint.selectPrinter({})
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
        <Button onPress={this.printHTML} title="Print HTML" disabled={Platform.OS === 'windows'} />
        <Button onPress={this.printPDF} title="Print PDF" disabled={Platform.OS === 'windows'} />
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
