/*------------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *------------------------------------------------------------------------*/

package com.aicas.xmpp;

public class DataPoint
{
  /*
    UID=00001|
    UNM=B01 - Van|
    LOC=EUR Logistics|
    ACT=Delivering parts to customers in London.|
    GEO=51.523127551944754+-0.0763472695017039|
    DID=00003|
    DNM=Fuel Gauge|
    KEY=&#37;|
    TYP=horizontal|
    MRK=0+red+10+amber+30+green+70+amber+90+red+100|
    VAL=43.68330972592727
  */
  /*
    UID = Unique Unit ID Number
    UNM = Unit Name (text) ...used to identify this unit individually.
    LOC = Parent Name (text) ...used to identify this unit's parent site or fleet (eg. "NA Fleet", "Plant One" or "US Headquarters").
    ACT = Current Action (text) ...used to explain the current action this unit is taking (eg. "Returning to NYC depot" or "Parked at base").
    GEO = Co-ordinates (latitude+longitude) ...lat/lon position of the unit.
    DID = Unique Device ID Number
    DNM = Device Name (text)
    KEY = Device Key (escaped HTML text) ...measurement symbol this device is reporting (eg. "&#37;" = percent, "&deg;C" = degrees in celcius, and "m&sup3;" = meters cubed)
    TYP = Gauge Type (selection) ...define what gauge type is required (options include "horizontal", "vertical", "circular" and "switch") NB. "id" and "tracker" are automatically added as the first two gauges.
    MRK = Gauge Ranges (custom) ...this data constructs the visual gauge ranges (see below for more infromation on this).
    VAL = The Value (number) ...this is the actual value being sent by the device (where a device is a switch or a flag, binary values are used - eg. zero = off, or stop, and one = on or go etc).
  */
  final int uid_;
  final String unm_;
  final String loc_;
  final String act_;
  final double geo_latitude_;
  final double geo_longitude_;
  final int did_;
  final String dnm_;
  final String key_;
  final String typ_;
  final double mrk1_;
  final String mrkColor1_;
  final double mrk2_;
  final String mrkColor2_;
  final double mrk3_;
  final String mrkColor3_;
  final double mrk4_;
  final String mrkColor4_;
  final double mrk5_;
  final String mrkColor5_;
  final double val_;

  DataPoint(int uid, String unm, String loc, String act, double geo_latitude,
            double geo_longitude, int did, String dnm, String key, String typ,
            double mrk1, String mrkColor1,
            double mrk2, String mrkColor2,
            double mrk3, String mrkColor3,
            double mrk4, String mrkColor4,
            double mrk5, String mrkColor5,
            double val)
  {
    this.uid_ = uid;
    this.unm_ = unm;
    this.loc_ = loc;
    this.act_ = act;
    this.geo_latitude_ = geo_latitude;
    this.geo_longitude_ = geo_longitude;
    this.did_ = did;
    this.dnm_ = dnm;
    this.key_ = key;
    this.typ_ = typ;
    this.mrk1_ = mrk1;
    this.mrkColor1_ = mrkColor1;
    this.mrk2_ = mrk2;
    this.mrkColor2_ = mrkColor2;
    this.mrk3_ = mrk3;
    this.mrkColor3_ = mrkColor3;
    this.mrk4_ = mrk4;
    this.mrkColor4_ = mrkColor4;
    this.mrk5_ = mrk5;
    this.mrkColor5_ = mrkColor5;
    this.val_ = val;
  }

  public String messageBody()
  {
    return "UID=" + uid_ + "|" +
           "UNM=" + unm_ + "|" +
           "LOC=" + loc_ + "|" +
           "ACT=" + act_ + "|" +
           "GEO=" + geo_latitude_ + "+" + geo_longitude_ + "|" +
           "DID=" + did_ + "|" +
           "DNM=" + dnm_ + "|" +
           "KEY=" + key_ + "|" +
           "TYP=" + typ_ + "|" +
           "MRK=" + mrk1_ + "+" + mrkColor1_ + "+"
                  + mrk2_ + "+" + mrkColor2_ + "+"
                  + mrk3_ + "+" + mrkColor3_ + "+"
                  + mrk4_ + "+" + mrkColor4_ + "+"
                  + mrk5_ + "+" + mrkColor5_ + "|" +
           "VAL=" + val_;
  }
}
