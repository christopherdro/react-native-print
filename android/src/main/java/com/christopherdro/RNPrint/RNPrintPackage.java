package com.christopherdro.RNPrint;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.facebook.react.ReactPackage;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.uimanager.ViewManager;
import com.facebook.react.bridge.JavaScriptModule;

public class RNPrintPackage implements ReactPackage {

  @Override
  public List<NativeModule> createNativeModules(
                              ReactApplicationContext reactContext) {
    List<NativeModule> modules = new ArrayList<NativeModule>();

    modules.add(new RNPrintModule(reactContext));

    return modules;
  }

  @Override
  public List<ViewManager> createViewManagers(
                            ReactApplicationContext reactContext) {
  	return Collections.emptyList();
  }

}
