del %1
copy bin\crystalpicnic-project-release-unsigned.apk tmp.apk
c:\Progra~1\Java\jdk1.8.0_05\bin\jarsigner.exe -verbose -sigalg MD5withRSA -digestalg SHA1 -keystore my-release-key.keystore tmp.apk Nooskewl
c:\Progra~1\Java\jdk1.8.0_05\bin\jarsigner.exe -verify tmp.apk
c:\users\trent\AppData\local\Android\android-sdk\build-tools\19.1.0\zipalign -v 4 tmp.apk %1
del tmp.apk
