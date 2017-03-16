/*-----------------------------------------------------------------------*
 * Copyright 2016, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *-----------------------------------------------------------------------*/
package com.aicas.xmpp;

/**
 * A message.
 */
public interface Message
{
  /**
   * Get the useful content.
   * @return the message's body
   */
  String getBody();
}
