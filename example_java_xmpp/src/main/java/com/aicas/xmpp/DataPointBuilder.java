/*------------------------------------------------------------------------*
 * Copyright 2017, aicas GmbH; all rights reserved.
 * This header, including copyright notice, may not be altered or removed.
 *------------------------------------------------------------------------*/

package com.aicas.xmpp;

public class DataPointBuilder {
  private int uid_;
  private String unm_;
  private String loc_;
  private String act_;
  private double geo_latitude_;
  private double geo_longitude_;
  private int did_;
  private String dnm_;
  private String key_;
  private String typ_;
  private double mrk1_;
  private String mrkColor1_;
  private double mrk2_;
  private String mrkColor2_;
  private double mrk3_;
  private String mrkColor3_;
  private double mrk4_;
  private String mrkColor4_;
  private double mrk5_;
  private String mrkColor5_;
  private double val_;


  public DataPoint createDataPoint() {
      return new DataPoint(uid_, unm_, loc_, act_, geo_latitude_,
                           geo_longitude_, did_, dnm_, key_, typ_,
                           mrk1_, mrkColor1_, mrk2_, mrkColor2_,
                           mrk3_, mrkColor3_, mrk4_, mrkColor4_,
                           mrk5_, mrkColor5_, val_);
  }


  public DataPointBuilder setUid(int uid_)
  {
      this.uid_ = uid_;
      return this;
  }


  public DataPointBuilder setUnm(String unm_)
  {
      this.unm_ = unm_;
      return this;
  }


  public DataPointBuilder setLoc(String loc_)
  {
      this.loc_ = loc_;
      return this;
  }


  public DataPointBuilder setAct(String act_)
  {
      this.act_ = act_;
      return this;
  }


  public DataPointBuilder setGeo_latitude(double geo_latitude_)
  {
      this.geo_latitude_ = geo_latitude_;
      return this;
  }


  public DataPointBuilder setGeo_longitude(double geo_longitude_)
  {
      this.geo_longitude_ = geo_longitude_;
      return this;
  }


  public DataPointBuilder setDid(int did_)
  {
      this.did_ = did_;
      return this;
  }


  public DataPointBuilder setDnm(String dnm_)
  {
      this.dnm_ = dnm_;
      return this;
  }


  public DataPointBuilder setKey(String key_)
  {
      this.key_ = key_;
      return this;
  }


  public DataPointBuilder setTyp(String typ_)
  {
      this.typ_ = typ_;
      return this;
  }


  public DataPointBuilder setMrk1(double mrk1_)
  {
      this.mrk1_ = mrk1_;
      return this;
  }


  public DataPointBuilder setMrkColor1(String mrkColor1_)
  {
      this.mrkColor1_ = mrkColor1_;
      return this;
  }


  public DataPointBuilder setMrk2(double mrk2_)
  {
      this.mrk2_ = mrk2_;
      return this;
  }


  public DataPointBuilder setMrkColor2(String mrkColor2_)
  {
      this.mrkColor2_ = mrkColor2_;
      return this;
  }


  public DataPointBuilder setMrk3(double mrk3_)
  {
      this.mrk3_ = mrk3_;
      return this;
  }


  public DataPointBuilder setMrkColor3(String mrkColor3_)
  {
      this.mrkColor3_ = mrkColor3_;
      return this;
  }


  public DataPointBuilder setMrk4(double mrk4_)
  {
      this.mrk4_ = mrk4_;
      return this;
  }


  public DataPointBuilder setMrkColor4(String mrkColor4_)
  {
      this.mrkColor4_ = mrkColor4_;
      return this;
  }


  public DataPointBuilder setMrk5(double mrk5_)
  {
      this.mrk5_ = mrk5_;
      return this;
  }


  public DataPointBuilder setMrkColor5(String mrkColor5_)
  {
      this.mrkColor5_ = mrkColor5_;
      return this;
  }


  public DataPointBuilder setVal(double val_)
  {
      this.val_ = val_;
      return this;
  }


}