/*******************************************************************************
*
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2014 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: application_poll_notification.h
*
* Abstract:  Provides a mechanism for the application to register for an receive
*            callbacks from the FW when a transmit opportinity is coming up.
*
*******************************************************************************/
#include "types.h"
#include "cfa.h"

#ifndef _APPLICATION_POLL_NOTIFICATION_H_
#define _APPLICATION_POLL_NOTIFICATION_H_


/// Initialize the connection event notification notification mechanism. Needs to be done once in
/// the app_create function.
void blecm_connectionEventNotifiationInit(void);


/// Register for notifications from the baseband of upcoming TX opportunities.
/// \param clientCallback The callback to be invoked.
/// \param clientContext The context (the UINT32 parameter to clientCallback) that must be passed
///                                 back to the callback.
/// \param offset Number of BT slots before/after the TX opportunity to call the callback. When negative,
///                    callback will be invoked before TX and when positive, callback will be invoked after the TX.
///                    This has to be an even number of BT slots (multiples of 1.25mS).
/// \param defaultPeriod When not connected, period of the callback in BT slots. Has to be an even number
///                     of slots. Don't set this to under ~5mS (8 slots).
/// \param connHandle - the connection handle of the connection for which we need the notifications.
void blecm_connectionEventNotifiationEnable(void (*clientCallback)(void*, UINT32), UINT32 clientContext,
                                            INT16 offset, UINT16 defaultPeriod, UINT32 connHandle);


/// Stop the connection event notification.
void blecm_connectionEventNotifiationDisable(void);

#endif  // _APPLICATION_POLL_NOTIFICATION_H_
