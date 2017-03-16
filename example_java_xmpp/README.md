Java XMPP Example
============

A simple example to connect to the aicas demo XMPP server

## Usage

### Build

- _./gradlew jar_  to build the project

### Include in other projects

- add the jars from _./lib_ and _./build/libs/exmaple-java-xmpp-1.0-SNAPSHOT.jar_ to the classpath

### Usage example
``` SendRobotData.sendMovement("base",   //name of the joint. So base, shoulder, elbow or wirst.
                               "NORMAL", //name of the mode. So NORMAL or RELATIV, MEM_POS or POS_MEM.
                               90,       //angle
                               1,        //increment 
                               10);      //delay
```
- the first call to one of the static methods will establish a connection to the aicas demo XMPP server,
 all following calls will use the same connection

### GUI example

- will open a small window to send test messages
- start it with:
```
 java -cp lib/jxmpp-core-0.4.2.jar:lib/jxmpp-util-cache-0.4.2.jar:lib/smack-core-4.1.9.jar:lib/smack-extensions-4.1.9.jar:lib/smack-im-4.1.9.jar:lib/smack-java7-4.1.9.jar:lib/smack-sasl-provided-4.1.9.jar:lib/smack-tcp-4.1.9.jar:lib/xpp3-1.1.4c.jar:build/libs/exmaple-java-xmpp-1.0-SNAPSHOT.jar com.aicas.xmpp.GUISendingExample_
```
- the _Setup receiver_ button creates a separate connection to test if the multiuser chat works, any messages get printed to the console.
