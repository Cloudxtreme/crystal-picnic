rm $1
cp bin/crystalpicnic-project-release-unsigned.apk tmp.apk
jarsigner -verbose -sigalg MD5withRSA -digestalg SHA1 -keystore ../../my-release-key.keystore tmp.apk Nooskewl
jarsigner -verify tmp.apk
~/code/android-sdk-linux/build-tools/23.0.2/zipalign -v 4 tmp.apk $1
rm tmp.apk
