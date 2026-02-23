====================
Deploying to Android
====================

Install Java JDK
----------------

1. Install the latest JDK:

	* https://www.oracle.com/java/technologies/downloads/

2. Set JAVA_HOME environment variable, if the JDK installer did not:

.. code::

	export JAVA_HOME=<java_jdk>

Install Android SDK & NDK
-------------------------

1. Install sdkmanager:

	* https://developer.android.com/tools/sdkmanager

2. Install Android SDK, NDK and tools:

.. code::

	<android_sdk>/latest/cmdline-tools/sdkmanager \
		"platforms;android-33" \
		"ndk;27.2.12479018" \
		"platform-tools" \
		"build-tools;34.0.0"

build-tools >= 34.0.0 is required.

Set environment variables
-------------------------

.. code::

   export ANDROID_SDK_PATH=<android_sdk>
   export ANDROID_API_LEVEL=33
   export ANDROID_NDK_ROOT=<android_ndk>
   export ANDROID_NDK_ABI=24
   export ANDROID_BUILD_TOOLS=<android_build-tools>/34.0.0

Generate a keystore
-------------------

.. code::

	keytool -genkey -v -keystore test.keystore -alias test \
		-keyalg RSA -keysize 2048 -validity 10000 \
		-keypass 123456 -storepass 123456
