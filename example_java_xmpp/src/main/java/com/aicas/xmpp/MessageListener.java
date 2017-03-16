/*-----------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *-----------------------------------------------------------------------*/
package com.aicas.xmpp;

import java.util.function.Consumer;

/**
 * Tagging interface to implement to read messages.
 */
public interface MessageListener extends Consumer<Message>
{
}
