/*-----------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *-----------------------------------------------------------------------*/
package com.aicas.xmpp;

import org.jivesoftware.smack.packet.Stanza;

public class XMPPMessage implements Message
{
  private org.jivesoftware.smack.packet.Message message_;

  XMPPMessage(Stanza stanza)
  {
    this.message_ = (org.jivesoftware.smack.packet.Message) stanza;
  }

  public String getBody()
  {
    return message_.getBody();
  }
}
