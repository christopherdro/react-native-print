require 'json'
pjson = JSON.parse(File.read('package.json'))

Pod::Spec.new do |s|
  s.name            = pjson["name"]
  s.version         = pjson["version"]
  s.homepage        = "https://github.com/christopherdro/react-native-print"
  s.summary         = pjson["description"]
  s.license         = pjson["license"]
  s.author          = { "Christopher Dro" => "casheghian@gmail.com" }

  s.requires_arc   = true
  s.platform       = :ios, '7.0'

  s.source          = { :git => "https://github.com/christopherdro/react-native-print", :tag => "v#{s.version}" }
  s.source_files    = 'ios/**/*.{h,m}'

  s.dependency 'React-Core'
end
