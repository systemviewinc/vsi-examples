/*-----------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *-----------------------------------------------------------------------*/
package com.aicas.xmpp;

import org.jivesoftware.smack.SmackException.NoResponseException;
import org.jivesoftware.smack.SmackException.NotConnectedException;
import org.jivesoftware.smack.XMPPConnection;
import org.jivesoftware.smack.XMPPException.XMPPErrorException;
import org.jivesoftware.smack.packet.Message;
import org.jivesoftware.smackx.muc.MultiUserChatManager;

public class XMPPMultiUserChat
{
  private final String nickname_;
  private org.jivesoftware.smackx.muc.MultiUserChat multiUserChat_;

  public XMPPMultiUserChat(XMPPConnection connection,
                           String jabberRoomId,
                           String nickname)
  {
    MultiUserChatManager multiUserChatManager = MultiUserChatManager.getInstanceFor(connection);
    multiUserChat_ = multiUserChatManager.getMultiUserChat(jabberRoomId);
    nickname_ = nickname;
  }

  public boolean join(final MessageListener messageListener)
  {
    boolean result = false;
    try
      {
        multiUserChat_.join(nickname_);
        if (messageListener != null)
          {
            multiUserChat_.addMessageListener(new org.jivesoftware.smack.MessageListener()
            {
              //@Override
              public void processMessage(Message message)
              {
                messageListener.accept(new XMPPMessage(message));
              }
            });
          }
        result = true;
      }
    catch (NoResponseException e)
      {
        e.printStackTrace();
      }
    catch (XMPPErrorException e)
      {
        e.printStackTrace();
      }
    catch (NotConnectedException e)
      {
        e.printStackTrace();
      }
    return result;
  }

  public boolean leave()
  {
    boolean result = false;
    try
      {
        multiUserChat_.leave();
        result = true;
      }
    catch (NotConnectedException e)
      {
        // Nothing we can do
      }
    return result;
  }

  //@Override
  public boolean isJoined()
  {
    return multiUserChat_.isJoined();
  }

  public boolean send(String message)
  {
    boolean result = false;
    if (isJoined())
      {
        System.out.println("send message = " + message);
        try
          {
            multiUserChat_.sendMessage(message);
          }
        catch (NotConnectedException e)
          {
            e.printStackTrace();
          }
        result = true;
      }
    else
      {
        System.out.println("Multiuserchat not connected.");
      }
    return result;
  }
}
