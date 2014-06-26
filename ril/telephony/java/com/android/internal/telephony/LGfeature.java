package com.android.internal.telephony;

import android.content.Context;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

public class LGfeature
{
  public static final int ATANDT = 4;
  public static final int DCMSET = 3;
  public static final int KT = 5;
  public static final int LGUPLUS = 2;
  static final String LOG_TAG = "LGfeature";
  public static final int MPCS = 7;
  public static final int MPDN_NOTSUPPORT = 0;
  public static final int NAI_ALLSUPPORT = 0;
  public static final int NAI_IntAndAppV4 = 1;
  public static final int SKT = 6;
  public static final int VZWbaseSet = 1;
  public boolean APN_SELECTION = false;
  public boolean BLOCK_NEXTAPN = false;
  public boolean CLEAN_IP_TABLE_RULES = false;
  public boolean DELAY_FOR_IMS = false;
  public boolean DO_RESET_TIMER_FOR_MMS_APN = false;
  public boolean DefaultPDNdependancy = false;
  public boolean FAST_DORMANCY = false;
  public boolean FD_DELAY_CALLING = false;
  public boolean FOR_STREAMING_SERVICE = false;
  public int IMSPowerOffdelaytime = 0;
  public boolean IP_ADDR_CHANGED = false;
  public boolean IP_ADDR_CHANGED_INTENT_FOR_IMS = false;
  public boolean LCDon_PSretry = false;
  public boolean LGU_3G_ROAMING = false;
  public boolean LIMIT_DATA_USAGE = false;
  public boolean LOCK_ORDER_RECEIVED_1X = false;
  public boolean MANUAL_NET_SEARCH = false;
  public boolean MMS_MOBILE_OFF = false;
  public int MPDNset = 0;
  public int NAI_support = 0;
  public boolean NOTI_GPRS_REJECT = false;
  public boolean NoDisconnectforChangeIP = false;
  public boolean PAYPOPUP_KO = false;
  public boolean PDNsyncWithModem = false;
  public boolean PROTECTION_VOICE_CALL = false;
  public boolean PermanentFailRetry = false;
  public boolean RETURN_OTHERTYPE = false;
  public boolean SUPPORT_DEFAULT_PREFER_APN = false;
  public boolean SUPPORT_DOCOMO_ROAMING_MTU = false;
  public boolean SUPPORT_DOCOMO_TETHER = false;
  public boolean SUPPORT_INHIBIT_APN_CHANGE = false;
  public boolean SUPPORT_MODE_CHANGE = false;
  public boolean SUPPORT_NUMERIC = false;
  public boolean SUPPORT_SINGLE_PDN = false;
  public boolean SUPPORT_SINGLE_PDN_CDMA = false;
  public boolean Sending_MMS_3G_Block = false;
  public int cdmaTimer = 0;
  public boolean disallowed1xDataCall = false;
  public boolean fixthephonetypetoCDMA = false;
  private Context mContext;
  public String myfeatureset = null;
  public boolean poweroffdelayneed = false;
  public int retry_interval = 0;
  public boolean rxtx_debug = false;
  public int sigstrengthtype = 0;
  public boolean useCdmaTimer = false;
  public boolean usePcscfAddress = false;
  public boolean wifiduringtethering = false;

  public LGfeature(Context paramContext, String paramString)
  {
    this.myfeatureset = paramString;
    this.mContext = paramContext;
    if (Settings.Secure.getInt(this.mContext.getContentResolver(), "preferred_phone_type", 0) == 1)
    {
      Log.v("LGfeature", "fixed type is setting, your phone will be fixed to cdma ");
      this.fixthephonetypetoCDMA = true;
    }
    if (TextUtils.equals(paramString, "VZWBASE"))
    {
      this.MPDNset = 1;
      this.NAI_support = 1;
      this.retry_interval = 1;
      this.DefaultPDNdependancy = true;
      this.poweroffdelayneed = true;
      this.PDNsyncWithModem = true;
      this.IMSPowerOffdelaytime = 2000;
      this.usePcscfAddress = true;
      this.NoDisconnectforChangeIP = true;
      this.useCdmaTimer = true;
      this.cdmaTimer = 900000;
      this.sigstrengthtype = 1;
      return;
    }
    if (TextUtils.equals(paramString, "LGTBASE"))
    {
      this.MPDNset = 0;
      this.poweroffdelayneed = true;
      this.PDNsyncWithModem = false;
      this.IMSPowerOffdelaytime = 3000;
      this.NoDisconnectforChangeIP = true;
      this.sigstrengthtype = 2;
      this.disallowed1xDataCall = false;
      this.SUPPORT_MODE_CHANGE = false;
      this.SUPPORT_SINGLE_PDN_CDMA = false;
      this.SUPPORT_NUMERIC = false;
      this.PAYPOPUP_KO = true;
      this.FAST_DORMANCY = true;
      this.LOCK_ORDER_RECEIVED_1X = true;
      this.DELAY_FOR_IMS = true;
      this.Sending_MMS_3G_Block = true;
      this.IP_ADDR_CHANGED = true;
      this.IP_ADDR_CHANGED_INTENT_FOR_IMS = true;
      this.LGU_3G_ROAMING = true;
      this.RETURN_OTHERTYPE = true;
      this.FOR_STREAMING_SERVICE = true;
      this.DO_RESET_TIMER_FOR_MMS_APN = true;
      this.CLEAN_IP_TABLE_RULES = true;
      this.wifiduringtethering = true;
      return;
    }
    if (TextUtils.equals(paramString, "ATANDT"))
    {
      this.MPDNset = 4;
      this.PDNsyncWithModem = true;
      this.sigstrengthtype = 4;
      return;
    }
    if (TextUtils.equals(paramString, "KTBASE"))
    {
      this.LCDon_PSretry = true;
      this.PermanentFailRetry = false;
      this.sigstrengthtype = 5;
      this.PAYPOPUP_KO = true;
      this.RETURN_OTHERTYPE = true;
      this.PROTECTION_VOICE_CALL = true;
      this.wifiduringtethering = true;
      return;
    }
    if (TextUtils.equals(paramString, "SKTBASE"))
    {
      this.PAYPOPUP_KO = true;
      this.MMS_MOBILE_OFF = true;
      this.APN_SELECTION = true;
      this.BLOCK_NEXTAPN = true;
      this.NOTI_GPRS_REJECT = true;
      this.PROTECTION_VOICE_CALL = true;
      this.MANUAL_NET_SEARCH = true;
      this.LIMIT_DATA_USAGE = true;
      this.sigstrengthtype = 6;
      this.wifiduringtethering = true;
      this.FD_DELAY_CALLING = true;
      this.RETURN_OTHERTYPE = true;
      return;
    }
    if (TextUtils.equals(paramString, "DCMBASE"))
    {
      this.SUPPORT_INHIBIT_APN_CHANGE = true;
      this.SUPPORT_DEFAULT_PREFER_APN = true;
      this.SUPPORT_DOCOMO_TETHER = true;
      this.SUPPORT_DOCOMO_ROAMING_MTU = true;
      this.RETURN_OTHERTYPE = true;
      return;
    }
    if (TextUtils.equals(paramString, "MPCS"))
    {
      this.sigstrengthtype = 7;
      return;
    }
    Log.i("LGfeature", "Wrong feature");
  }

  public void fixPhoneType(boolean paramBoolean)
  {
    if (paramBoolean)
    {
      Settings.Secure.putInt(this.mContext.getContentResolver(), "preferred_phone_type", 1);
      return;
    }
    Settings.Secure.putInt(this.mContext.getContentResolver(), "preferred_phone_type", 0);
  }

  public String toString()
  {
    StringBuilder localStringBuilder = new StringBuilder();
    localStringBuilder.append("my featureset: " + this.myfeatureset).append(" MPDN: ").append(this.MPDNset).append(" NAI_SUPPORT: ").append(this.NAI_support).append(" poweroffdelayneed ").append(this.poweroffdelayneed);
    return localStringBuilder.toString();
  }
}