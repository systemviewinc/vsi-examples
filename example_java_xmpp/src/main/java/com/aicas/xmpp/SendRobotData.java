/*-----------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *-----------------------------------------------------------------------*/
package com.aicas.xmpp;

import java.io.IOException;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.util.Properties;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import org.jivesoftware.smack.ConnectionConfiguration;
import org.jivesoftware.smack.SmackException;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.java7.Java7SmackInitializer;
import org.jivesoftware.smack.packet.Presence;
import org.jivesoftware.smack.tcp.XMPPTCPConnection;
import org.jivesoftware.smack.tcp.XMPPTCPConnectionConfiguration;

public class SendRobotData
{
  private static SendRobotData instance = null;
  private static SendRobotData receiver = null;
  private boolean mDebugXMPP_;
  private ConnectionConfiguration.SecurityMode mSecurityMode_;
  private String mHost_;
  private String mService_;
  private String mResource_;
  private String mPassword_;
  private String mUsername_;
  private boolean mConnected_ = false;
  private String mNickName_;
  private String mGroupChatName_;
  private String mGroupChatJid_;
  private XMPPMultiUserChat mXMPPMultiUserChat_;
  private XMPPTCPConnection mXmppConnection_;
  private XMPPTCPConnectionConfiguration mXMPPTCPConnectionConfiguration_;

  private SendRobotData(boolean isReceiver)
  {

    Properties prop = new Properties();
    InputStream in = SendRobotData.class.getResourceAsStream("/xmpp.properties");
    String receiver = "";
    if (isReceiver) {
      receiver = "-receiver-test";
    }
    try
      {
        prop.load(in);
      }
    catch (IOException e)
      {
        e.printStackTrace();
      }
    mHost_ = prop.getProperty("xmpp.host");
    mService_ = prop.getProperty("xmpp.service");
    mResource_ = prop.getProperty("xmpp.resource") + receiver;
    mPassword_ = prop.getProperty("xmpp.password");
    mUsername_ = prop.getProperty("xmpp.username");
    mNickName_ = prop.getProperty("xmpp.nickname") + receiver;
    mGroupChatName_ = prop.getProperty("xmpp.groupChatName");
    mGroupChatJid_ = mGroupChatName_ + "@conference." + mService_;
    mDebugXMPP_ = Boolean.valueOf(prop.getProperty("xmpp.debugXMPP"));
    mSecurityMode_ = ConnectionConfiguration.SecurityMode.valueOf(prop.getProperty("xmpp.securityMode"));

    mXMPPTCPConnectionConfiguration_ = configureXMPPConnection(mHost_,
                                                               mUsername_,
                                                               mPassword_,
                                                               mService_,
                                                               mDebugXMPP_,
                                                               mResource_,
                                                               mSecurityMode_);

    try
      {
        connect();
      }
    catch (IOException e)
      {
        e.printStackTrace();
      }
    catch (XMPPException e)
      {
        e.printStackTrace();
      }
    catch (SmackException e)
      {
        e.printStackTrace();
      }
    mXMPPMultiUserChat_ = new XMPPMultiUserChat(mXmppConnection_,
                                                mGroupChatJid_,
                                                mNickName_);

    mXMPPMultiUserChat_.join(new MessageListener()
    {
      @Override
      public void accept(Message message)
      {
        System.out.println("message.getBody() = " + message.getBody());
      }
    });
  }

  public static synchronized SendRobotData getInstance()
  {
    if (instance == null)
      {
        instance = new SendRobotData(false);
      }
    return instance;
  }

  public static void setupReceiver() {
    if (receiver == null)
      {
        receiver = new SendRobotData(true);
      }
  }

  public static boolean sendMovement(String name, String mode, int angle, int incr, int delay)
  {
    DataPoint dataPoint = new DataPointBuilder().setUid(1)
                                                .setUnm("iRobot")
                                                .setLoc("EmbeddedWorld")
                                                .setDnm(name)
                                                .setAct(mode)
                                                .setDid(4)
                                                .setMrk1(angle)
                                                .setMrk2(incr)
                                                .setMrk3(delay)
                                                .createDataPoint();

    return getInstance().mXMPPMultiUserChat_.send(dataPoint.messageBody());
  }

  public static boolean calibrate()
  {
    DataPoint dataPoint = new DataPointBuilder().setUid(1)
                                                .setUnm("iRobot")
                                                .setLoc("EmbeddedWorld")
                                                .setAct("Positioning")
                                                .setDnm("Calibrate")
                                                .setDid(0)
                                                .setVal(1)
                                                .createDataPoint();
    return getInstance().mXMPPMultiUserChat_.send(dataPoint.messageBody());
  }

  private void connect() throws IOException, XMPPException, SmackException
  {
    if (mConnected_)
      {
        return;
      }
    new Java7SmackInitializer().initialize();
    mXmppConnection_ = new XMPPTCPConnection(mXMPPTCPConnectionConfiguration_);
    mXmppConnection_.setPacketReplyTimeout(30_000);
    mXmppConnection_.connect();
    if (mXmppConnection_.isConnected())
      {
        System.out.println("connected");
      }
    mXmppConnection_.login();
    mConnected_ = true;
    if (mXmppConnection_.isAuthenticated())
      {
        System.out.println("authenticated");
      }

    Presence presence = new Presence(Presence.Type.available);
    presence.setPriority(2);
    mXmppConnection_.sendStanza(presence);
  }

  private XMPPTCPConnectionConfiguration configureXMPPConnection(String host,
                                                                 String username,
                                                                 String password,
                                                                 String service,
                                                                 boolean debugXMPP,
                                                                 String resource,
                                                                 ConnectionConfiguration.SecurityMode securityMode)
  {
//    SSLContext context;
//    try
//      {
//        KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
//        trustStore.load(SendRobotData.class.getClassLoader().getResourceAsStream("trust.jks"), "blabla".toCharArray());
//        TrustManagerFactory trustFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
//        trustFactory.init(trustStore);
//
//        KeyStore identityStore = KeyStore.getInstance(KeyStore.getDefaultType());
//        identityStore.load(SendRobotData.class.getClassLoader().getResourceAsStream("client10.jks"),
//                           "blabla".toCharArray());
//        KeyManagerFactory keyFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
//        keyFactory.init(identityStore, "blabla".toCharArray());
//
//        context = SSLContext.getInstance("TLSv1.2");
//        context.init(keyFactory.getKeyManagers(), trustFactory.getTrustManagers(), new SecureRandom());
//      }
//    catch (Exception e)
//      {
//        e.printStackTrace();
//      }

    return XMPPTCPConnectionConfiguration.builder()
                                         .setHost(host)
                                         .setServiceName(service)
                                         .setUsernameAndPassword(username, password)
                                         .setResource(resource)
                                         .setDebuggerEnabled(debugXMPP)
                                         .setSecurityMode(securityMode)
                                         .build();
  }
}
