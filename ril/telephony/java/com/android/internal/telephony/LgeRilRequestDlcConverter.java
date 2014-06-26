package com.android.internal.telephony;

import android.util.Log;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

public class LgeRilRequestDlcConverter
{
  static final int LGE_MAX_DLC_CHANNELS = 64;
  static final String LOG_TAG = "RILJ";
  static HashMap<String, Integer> mIpcChannelDlcMappingContainer = new HashMap();
  static int mMaxDlcNumber = 0;

  public LgeRilRequestDlcConverter()
  {
    if (mIpcChannelDlcMappingContainer.isEmpty())
      readDlcMapping4IpcConfig();
  }

  static void dumpIpcChannelDlcMapping()
  {
    Iterator localIterator = mIpcChannelDlcMappingContainer.entrySet().iterator();
    while (localIterator.hasNext())
    {
      Map.Entry localEntry = (Map.Entry)localIterator.next();
      Log.d("RILJ", (String)localEntry.getKey() + " = " + localEntry.getValue());
    }
  }

  static int getDataDlcBasedOnCid(int paramInt1, int paramInt2, int paramInt3)
  {
    int i = paramInt2;
    RilIpcGroup localRilIpcGroup = RilIpcGroup.getDataControlGroupBasedCid(paramInt1);
    if (mIpcChannelDlcMappingContainer.isEmpty())
      readDlcMapping4IpcConfig();
    dumpIpcChannelDlcMapping();
    if (mIpcChannelDlcMappingContainer.containsKey(getIpcName(localRilIpcGroup)))
      i = ((Integer)mIpcChannelDlcMappingContainer.get(getIpcName(localRilIpcGroup))).intValue();
    Log.d("RILJ", "getDataDlcBasedOnCid - Request : < " + RIL.requestToString(paramInt3) + " >" + ", Dlc id - " + i);
    return i;
  }

  static int getDlcRequestId(int paramInt)
  {
    RilIpcGroup localRilIpcGroup = getIpcGroupReqId(paramInt);
    if (mIpcChannelDlcMappingContainer.isEmpty())
      readDlcMapping4IpcConfig();
    boolean bool = mIpcChannelDlcMappingContainer.containsKey(getIpcName(localRilIpcGroup));
    int i = 0;
    if (bool)
      i = ((Integer)mIpcChannelDlcMappingContainer.get(getIpcName(localRilIpcGroup))).intValue();
    Log.d("RILJ", "getDlcRequestId - Request : < " + RIL.requestToString(paramInt) + " >" + ", Dlc id - " + i);
    return i;
  }

  private static RilIpcGroup getIpcGroupReqId(int paramInt)
  {
    switch (paramInt)
    {
    case 58:
    case 60:
    case 65:
    case 66:
    case 75:
    case 77:
    case 78:
    case 79:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 104:
    case 105:
    case 106:
    case 107:
    case 131:
    case 231:
    case 232:
    case 233:
    case 236:
    case 237:
    case 239:
    case 241:
    case 243:
    case 245:
    case 249:
    case 255:
    case 257:
    case 259:
    case 261:
    case 263:
    case 266:
    case 268:
    case 269:
    case 270:
    case 271:
    case 273:
    case 275:
    case 279:
    case 283:
    case 291:
    case 295:
    case 298:
    case 299:
    case 300:
    case 301:
    case 302:
    case 303:
    default:
      Log.d("RILJ", "Default group : < " + RIL.requestToString(paramInt) + " >");
      return RilIpcGroup.IPC_GROUP_NONE;
    case 10:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 24:
    case 31:
    case 32:
    case 40:
    case 49:
    case 50:
    case 52:
    case 53:
    case 54:
    case 72:
    case 108:
    case 111:
    case 112:
    case 113:
    case 130:
    case 137:
    case 138:
    case 142:
    case 143:
    case 156:
    case 157:
    case 180:
    case 181:
    case 182:
    case 189:
    case 190:
    case 191:
    case 192:
    case 195:
    case 196:
    case 197:
    case 198:
    case 216:
    case 217:
    case 218:
    case 219:
    case 240:
    case 246:
    case 247:
    case 248:
    case 252:
    case 265:
    case 287:
    case 288:
      return RilIpcGroup.IPC_GROUP_CALL_CONTROL;
    case 27:
    case 41:
    case 56:
    case 57:
    case 121:
    case 122:
    case 123:
    case 124:
    case 125:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 168:
    case 169:
    case 175:
    case 176:
    case 177:
    case 178:
    case 183:
    case 184:
    case 185:
    case 186:
    case 187:
    case 221:
    case 228:
      return RilIpcGroup.IPC_GROUP_DATACALL_CONTROL_1;
    case 238:
    case 289:
    case 290:
      return RilIpcGroup.IPC_GROUP_FACTORY_TEST;
    case 276:
      return RilIpcGroup.IPC_GROUP_INITIALIZATION;
    case 11:
    case 23:
    case 38:
    case 39:
    case 44:
    case 51:
    case 80:
    case 81:
    case 109:
    case 110:
    case 119:
    case 158:
    case 159:
    case 160:
    case 161:
    case 172:
    case 173:
    case 179:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
    case 210:
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 222:
    case 223:
    case 256:
    case 272:
    case 274:
      return RilIpcGroup.IPC_GROUP_ME_CONTROL_AND_STATUS;
    case 19:
    case 20:
    case 21:
    case 22:
    case 45:
    case 48:
    case 61:
    case 73:
    case 74:
    case 76:
    case 139:
    case 140:
    case 144:
    case 145:
    case 146:
    case 188:
    case 224:
    case 225:
    case 229:
    case 230:
    case 234:
    case 235:
    case 250:
    case 251:
    case 253:
    case 254:
    case 284:
    case 285:
    case 286:
    case 294:
      return RilIpcGroup.IPC_GROUP_NETWORK;
    case 46:
    case 47:
      return RilIpcGroup.IPC_GROUP_NETWORK_REG;
    case 116:
    case 134:
    case 135:
    case 136:
    case 164:
    case 242:
    case 262:
    case 264:
    case 267:
      return RilIpcGroup.IPC_GROUP_PHONEBOOK;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 28:
    case 42:
    case 43:
    case 59:
    case 118:
    case 165:
    case 174:
    case 193:
    case 194:
    case 220:
    case 244:
    case 258:
    case 260:
    case 280:
    case 281:
    case 282:
    case 292:
    case 293:
    case 296:
    case 297:
    case 304:
      return RilIpcGroup.IPC_GROUP_SIM;
    case 25:
    case 26:
    case 37:
    case 63:
    case 64:
    case 102:
    case 126:
    case 127:
    case 128:
    case 129:
    case 141:
    case 155:
    case 166:
    case 167:
    case 199:
    case 277:
      return RilIpcGroup.IPC_GROUP_SMS;
    case 9:
    case 29:
    case 30:
    case 33:
    case 34:
    case 35:
    case 36:
    case 55:
    case 62:
    case 114:
    case 115:
    case 132:
    case 133:
    case 147:
    case 200:
    case 201:
    case 226:
    case 227:
      return RilIpcGroup.IPC_GROUP_SS_USSD;
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 103:
    case 120:
    case 202:
      return RilIpcGroup.IPC_GROUP_STK;
    case 278:
      return RilIpcGroup.IPC_GROUP_TEMP;
    case 117:
    case 162:
    case 163:
    case 170:
    case 171:
    }
    return RilIpcGroup.IPC_GROUP_UNSOLICITED;
  }

  static String getIpcName(RilIpcGroup paramRilIpcGroup)
  {
    return paramRilIpcGroup.toString();
  }

  static void readDlcMapping4IpcConfig()
  {
    LgeINIConfig localLgeINIConfig = new LgeINIConfig();
    int i = 0;
    HashMap localHashMap = new HashMap();
    if (localLgeINIConfig.lgeProfileRead("/system/etc/ipc_channels.config") > 0)
    {
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_NONE), Integer.valueOf(0));
      RilIpcGroup[] arrayOfRilIpcGroup = RilIpcGroup.values();
      int j = arrayOfRilIpcGroup.length;
      int k = 0;
      if (k < j)
      {
        RilIpcGroup localRilIpcGroup = arrayOfRilIpcGroup[k];
        String str = localLgeINIConfig.lgeProfileString("RIL_BASE", getIpcName(localRilIpcGroup), "");
        int n;
        if (!str.equals(""))
        {
          int m = str.indexOf(",");
          if (m > 0)
            str = str.substring(0, m);
          if ((!localHashMap.isEmpty()) && (localHashMap.containsKey(str)))
            break label179;
          localHashMap.put(str.trim(), Integer.valueOf(i));
          n = i;
          i++;
        }
        while (true)
        {
          mIpcChannelDlcMappingContainer.put(getIpcName(localRilIpcGroup), Integer.valueOf(n));
          k++;
          break;
          label179: n = ((Integer)localHashMap.get(str)).intValue();
        }
      }
    }
    for (mMaxDlcNumber = i; ; mMaxDlcNumber = 16)
    {
      localLgeINIConfig.lgeProfileFree();
      dumpIpcChannelDlcMapping();
      return;
      Log.e("RILJ", "ipc_channels.config file is missing. Using default HSI config");
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_NONE), Integer.valueOf(0));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_UNSOLICITED), Integer.valueOf(0));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_INITIALIZATION), Integer.valueOf(0));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_SIM), Integer.valueOf(1));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_STK), Integer.valueOf(1));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_CALL_CONTROL), Integer.valueOf(2));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_NETWORK), Integer.valueOf(3));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_SMS), Integer.valueOf(4));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_SS_USSD), Integer.valueOf(2));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_PHONEBOOK), Integer.valueOf(5));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_FACTORY_TEST), Integer.valueOf(6));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_ME_CONTROL_AND_STATUS), Integer.valueOf(0));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_TEMP), Integer.valueOf(0));
      mIpcChannelDlcMappingContainer.put(getIpcName(RilIpcGroup.IPC_GROUP_DATACALL_CONTROL_1), Integer.valueOf(7));
    }
  }

  int getMaxDlc()
  {
    return mMaxDlcNumber;
  }

  static enum RilIpcGroup
  {
    static
    {
      IPC_GROUP_INITIALIZATION = new RilIpcGroup("IPC_GROUP_INITIALIZATION", 1);
      IPC_GROUP_SIM = new RilIpcGroup("IPC_GROUP_SIM", 2);
      IPC_GROUP_STK = new RilIpcGroup("IPC_GROUP_STK", 3);
      IPC_GROUP_CALL_CONTROL = new RilIpcGroup("IPC_GROUP_CALL_CONTROL", 4);
      IPC_GROUP_NETWORK = new RilIpcGroup("IPC_GROUP_NETWORK", 5);
      IPC_GROUP_SMS = new RilIpcGroup("IPC_GROUP_SMS", 6);
      IPC_GROUP_SS_USSD = new RilIpcGroup("IPC_GROUP_SS_USSD", 7);
      IPC_GROUP_PHONEBOOK = new RilIpcGroup("IPC_GROUP_PHONEBOOK", 8);
      IPC_GROUP_FACTORY_TEST = new RilIpcGroup("IPC_GROUP_FACTORY_TEST", 9);
      IPC_GROUP_ME_CONTROL_AND_STATUS = new RilIpcGroup("IPC_GROUP_ME_CONTROL_AND_STATUS", 10);
      IPC_GROUP_TEMP = new RilIpcGroup("IPC_GROUP_TEMP", 11);
      IPC_GROUP_DATACALL_CONTROL_1 = new RilIpcGroup("IPC_GROUP_DATACALL_CONTROL_1", 12);
      IPC_GROUP_DATACALL_CONTROL_2 = new RilIpcGroup("IPC_GROUP_DATACALL_CONTROL_2", 13);
      IPC_GROUP_DATACALL_CONTROL_3 = new RilIpcGroup("IPC_GROUP_DATACALL_CONTROL_3", 14);
      IPC_GROUP_DATACALL_CONTROL_4 = new RilIpcGroup("IPC_GROUP_DATACALL_CONTROL_4", 15);
      IPC_GROUP_INTERNAL_REQUESTS = new RilIpcGroup("IPC_GROUP_INTERNAL_REQUESTS", 16);
      IPC_GROUP_NETWORK_REG = new RilIpcGroup("IPC_GROUP_NETWORK_REG", 17);
      IPC_GROUP_NONE = new RilIpcGroup("IPC_GROUP_NONE", 18);
      RilIpcGroup[] arrayOfRilIpcGroup = new RilIpcGroup[19];
      arrayOfRilIpcGroup[0] = IPC_GROUP_UNSOLICITED;
      arrayOfRilIpcGroup[1] = IPC_GROUP_INITIALIZATION;
      arrayOfRilIpcGroup[2] = IPC_GROUP_SIM;
      arrayOfRilIpcGroup[3] = IPC_GROUP_STK;
      arrayOfRilIpcGroup[4] = IPC_GROUP_CALL_CONTROL;
      arrayOfRilIpcGroup[5] = IPC_GROUP_NETWORK;
      arrayOfRilIpcGroup[6] = IPC_GROUP_SMS;
      arrayOfRilIpcGroup[7] = IPC_GROUP_SS_USSD;
      arrayOfRilIpcGroup[8] = IPC_GROUP_PHONEBOOK;
      arrayOfRilIpcGroup[9] = IPC_GROUP_FACTORY_TEST;
      arrayOfRilIpcGroup[10] = IPC_GROUP_ME_CONTROL_AND_STATUS;
      arrayOfRilIpcGroup[11] = IPC_GROUP_TEMP;
      arrayOfRilIpcGroup[12] = IPC_GROUP_DATACALL_CONTROL_1;
      arrayOfRilIpcGroup[13] = IPC_GROUP_DATACALL_CONTROL_2;
      arrayOfRilIpcGroup[14] = IPC_GROUP_DATACALL_CONTROL_3;
      arrayOfRilIpcGroup[15] = IPC_GROUP_DATACALL_CONTROL_4;
      arrayOfRilIpcGroup[16] = IPC_GROUP_INTERNAL_REQUESTS;
      arrayOfRilIpcGroup[17] = IPC_GROUP_NETWORK_REG;
      arrayOfRilIpcGroup[18] = IPC_GROUP_NONE;
    }

    static RilIpcGroup getDataControlGroupBasedCid(int paramInt)
    {
      RilIpcGroup localRilIpcGroup = IPC_GROUP_NONE;
      switch (paramInt)
      {
      default:
        return localRilIpcGroup;
      case 1:
        return IPC_GROUP_DATACALL_CONTROL_1;
      case 2:
        return IPC_GROUP_DATACALL_CONTROL_2;
      case 3:
        return IPC_GROUP_DATACALL_CONTROL_3;
      case 4:
      }
      return IPC_GROUP_DATACALL_CONTROL_4;
    }
  }
}