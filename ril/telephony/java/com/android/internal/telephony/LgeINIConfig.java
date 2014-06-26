package com.android.internal.telephony;

public class LgeINIConfig
{
  public static final String LGE_CONFIG_FILE_NAME = "/system/etc/ipc_channels.config";

  private native void native_profileFree();

  private native int native_profileHasSection(String paramString);

  private native int native_profileRead(String paramString);

  private native String native_profileString(String paramString1, String paramString2, String paramString3);

  protected void finalize()
    throws Throwable
  {
    native_profileFree();
    super.finalize();
  }

  public void lgeProfileFree()
  {
    native_profileFree();
  }

  public int lgeProfileHasSection(String paramString)
  {
    return native_profileHasSection(paramString);
  }

  public int lgeProfileRead(String paramString)
  {
    return native_profileRead(paramString);
  }

  public String lgeProfileString(String paramString1, String paramString2, String paramString3)
  {
    return native_profileString(paramString1, paramString2, paramString3);
  }
}