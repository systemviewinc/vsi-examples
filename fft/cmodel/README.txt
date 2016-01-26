

                    Core name: Xilinx LogiCORE Fast Fourier Transform C model
                    Version: 8.0
                    Release Date: September 21, 2010


================================================================================

This document contains the following sections: 

1. Introduction
2. New Features
3. Resolved Issues
4. Known Issues 
5. Technical Support
6. Other Information
7. Model Release History
8. Legal Disclaimer
 
================================================================================
 
1. INTRODUCTION

For the most recent updates to the IP installation instructions for this core,
please go to:

   http://www.xilinx.com/ipcenter/coregen/ip_update_install_instructions.htm

 
For system requirements:

   http://www.xilinx.com/ipcenter/coregen/ip_update_system_requirements.htm 


This file contains release notes for the Xilinx LogiCORE IP Fast Fourier Transform v8.0 
C Model. For the latest core updates, see the product page at:
 
  http://www.xilinx.com/products/ipcenter/FFT.htm


2. NEW FEATURES  
 
   - None
 
3. RESOLVED ISSUES 
 
   - N/A

4. KNOWN ISSUES 
   
   The following are known issues for v8.0 of this C model at time of release:

   - Slow model performance
     - The FFT bit accurate C model uses fixed point classes internally which make a number
       of malloc() and free() calls while the xfft_v8_0_bitacc_simulate() function is running.
       These kernel calls can impact the simulation speed of the model
     - CR471384

   - C model output incorrect for some 65536-point Pipelined, Streaming I/O FFTs with convergent rounding
     - When convergent rounding is used with a scaled or block floating point 64k Pipelined, Streaming I/O
       transform, some datasets can cause the final rounding operation performed by the model to produce
       values equal to +1.0, which cannot be represented by the IP core.  The IP core behaves correctly
       in these cases.
     - If the C model and IP core must match exactly in this case, using truncation rather than convergent
       rounding will yield the same results in both models.
     - CR476547

   The most recent information, including known issues, workarounds, and
   resolutions for this version is provided in the release notes Answer Record
   for the ISE 12.3 IP Update at 

   http://www.xilinx.com/support/documentation/user_guides/xtp025.pdf

5. TECHNICAL SUPPORT 

   To obtain technical support, create a WebCase at www.xilinx.com/support.
   Questions are routed to a team with expertise using this product.  
     
   Xilinx provides technical support for use of this product when used
   according to the guidelines described in the core documentation, and
   cannot guarantee timing, functionality, or support of this product for
   designs that do not follow specified guidelines.


6. OTHER INFORMATION
   
   Full documentation of the C model is provided in the document
   "xfft_bitacc_cmodel_ug459.pdf".


7. CORE RELEASE HISTORY 

Date        By            Version      Description
================================================================================
09/21/2010  Xilinx, Inc.  8.0          C model for FFT v8.0
04/19/2010  Xilinx, Inc.  7.1          C model for FFT v7.1
06/24/2009  Xilinx, Inc.  7.0          C model for FFT v7.0
09/19/2008  Xilinx, Inc.  6.0          C model for FFT v6.0
10/10/2007  Xilinx, Inc.  5.0          First release of C model, for FFT v5.0
================================================================================


8. Legal Disclaimer

(c) Copyright 2007 - 2010 Xilinx, Inc. All rights reserved. 

This file contains confidential and proprietary information
of Xilinx, Inc. and is protected under U.S. and
international copyright and other intellectual property
laws.

DISCLAIMER
This disclaimer is not a license and does not grant any
rights to the materials distributed herewith. Except as
otherwise provided in a valid license issued to you by
Xilinx, and to the maximum extent permitted by applicable
law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
(2) Xilinx shall not be liable (whether in contract or tort,
including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature
related to, arising under or in connection with these
materials, including for any direct, or any indirect,
special, incidental, or consequential loss or damage
(including loss of data, profits, goodwill, or any type of
loss or damage suffered as a result of any action brought
by a third party) even if such damage or loss was
reasonably foreseeable or Xilinx had been advised of the
possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-
safe, or for use in any application requiring fail-safe
performance, such as life-support or safety devices or
systems, Class III medical devices, nuclear facilities,
applications related to the deployment of airbags, or any
other applications that could lead to death, personal
injury, or severe property or environmental damage
(individually and collectively, "Critical
Applications"). Customer assumes the sole risk and
liability of any use of Xilinx products in Critical
Applications, subject only to applicable laws and
regulations governing limitations on product liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
PART OF THIS FILE AT ALL TIMES.

