package com.android.internal.telephony;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
//import com.android.internal.telephony.videotelephony.LgeVideoTelephonyIMS;

public abstract class LgeBaseCommands extends RIL
{
  static final String LGE_ENCODING_IRA = "IRA";
  static final String LGE_ENCODING_UCS2 = "UCS2";
  static final String LOG_TAG = "RILB";
  protected RegistrantList mLge1XRegDoneRegistrants = new RegistrantList();
  protected RegistrantList mLgeAocCCMmaxReportRegistrants = new RegistrantList();
  protected Registrant mLgeBatteryLevelUpdateRegistrant;
  protected RegistrantList mLgeCOLPRegistrants = new RegistrantList();
  protected RegistrantList mLgeCallControlInfoRegistrants = new RegistrantList();
  protected RegistrantList mLgeCdmaFwdBurstDtmfRegistrants = new RegistrantList();
  protected RegistrantList mLgeCdmaFwdContDtmfStartRegistrants = new RegistrantList();
  protected RegistrantList mLgeCdmaFwdContDtmfStopRegistrants = new RegistrantList();
  protected RegistrantList mLgeCdmaSidChangedRegistrants = new RegistrantList();
  protected RegistrantList mLgeCurrentCallMeterRegistrants = new RegistrantList();
  protected RegistrantList mLgeDataClosedByNetRegistrants = new RegistrantList();
  protected Registrant mLgeDeferredSmsReportRegistrant;
  protected RegistrantList mLgeDormancyRegistrant = new RegistrantList();
  protected RegistrantList mLgeDslgIfaceAddrChangedEv = new RegistrantList();
  protected RegistrantList mLgeDunBlockedRegistrants = new RegistrantList();
  protected RegistrantList mLgeDunFailedCauseOfAndroid3GDataRegistrants = new RegistrantList();
  protected Registrant mLgeDunRegistrant;
  protected Registrant mLgeEnablingFacilityLockRegistrant;
  protected RegistrantList mLgeEngineeringModeRegistrant = new RegistrantList();
  protected RegistrantList mLgeEvdoQoSChangedRegistrants = new RegistrantList();
  protected Registrant mLgeFactoryTestRegistrant;
  protected RegistrantList mLgeHdrSmpsChangedRegistrants = new RegistrantList();
  protected RegistrantList mLgeLockStateRegistrants = new RegistrantList();
  protected RegistrantList mLgeMtPsPageIndRegistrant = new RegistrantList();
  protected RegistrantList mLgeNetworkErrorDispRegistrants = new RegistrantList();
  protected Registrant mLgeNetworkErrorRegistrant;
  protected RegistrantList mLgeOtaSessionFailRegistrants = new RegistrantList();
  protected RegistrantList mLgeOtaSessionSuccessRegistrants = new RegistrantList();
  protected boolean mLgePbReady = false;
  protected Registrant mLgePbReadyRegistrant;
  protected boolean mLgeReportStkServiceIsRunning;
  protected Message mLgeReportStkServiceIsRunningMessage;
  protected RegistrantList mLgeSIMNotReadyRegistrants = new RegistrantList();
  protected Registrant mLgeSMSDCAbortConnectRegistrant;
  protected Registrant mLgeSMSDCConnectRegistrant;
  protected Registrant mLgeSMSDCDisConnectRegistrant;
  protected boolean mLgeSentPbReady = false;
  protected Registrant mLgeSmsMemStatusRegistrant;
  protected RegistrantList mLgeSmsValue = new RegistrantList();
  protected RegistrantList mLgeStkProactiveSessionStateRegistrants = new RegistrantList();
  protected Registrant mLgeUsimRemovedStatusRegistrant;
  protected Registrant mLgeUsimStateChangedRegistrant;
  protected RegistrantList mLgeVTPeerDisconnectRegistrant = new RegistrantList();
  protected RegistrantList mLgeVtCodecNegoResultRegistrant = new RegistrantList();
  protected RegistrantList mLgeVtFastFrameUpdateRegistrant = new RegistrantList();
  protected RegistrantList mLgeWpbxStateChangedRegistrants = new RegistrantList();
  protected Registrant mLgeXcallstatRegistrant;
  protected RegistrantList mLgecnapRegistrants = new RegistrantList();

  public LgeBaseCommands(Context paramContext, int paramInt1, int paramInt2, String paramString)
  {
    super(paramContext, paramInt1, paramInt2, paramString);
  }

  public void getLastPSVTCallFailCause(Message paramMessage)
  {
    // XXX
    /*if ((this.mLgeVtIms != null) && (true == this.mLgeVtIms.getLastCallFailCause(paramMessage)))
      riljLog("> RIL.java : getLastCallFailCause.................");*/
  }

  public boolean isPSVTCall()
  {
    return false;
  }

  public void lgeRegisterFor1XRegDone(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLge1XRegDoneRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForAocCCMmaxReport(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeAocCCMmaxReportRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForCNAP(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgecnapRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForCOLP(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeCOLPRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForCallControlInfo(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeCallControlInfoRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForCdmaSidChangedRegistrants(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeCdmaSidChangedRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForCurrentCallMeter(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeCurrentCallMeterRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForDormancyStatusChanged(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeDormancyRegistrant.add(localRegistrant);
  }

  public void lgeRegisterForDunBlocked(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeDunBlockedRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForEngineeringMode(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeEngineeringModeRegistrant.add(localRegistrant);
  }

  public void lgeRegisterForHdrSmpsChanged(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeHdrSmpsChangedRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForLockStateChanged(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeLockStateRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForMtPsPageInd(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeMtPsPageIndRegistrant.add(localRegistrant);
  }

  public void lgeRegisterForNetworkErrorDisp(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeNetworkErrorDispRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForNetworkRegistrationError(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeNetworkErrorRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeRegisterForOtaSessionFail(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeOtaSessionFailRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForOtaSessionSuccess(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeOtaSessionSuccessRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForPbReady(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgePbReadyRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    if (this.mLgePbReady)
    {
      this.mLgePbReadyRegistrant.notifyRegistrant(new AsyncResult(null, null, null));
      this.mLgePbReady = false;
    }
  }

  public void lgeRegisterForStkSessionState(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeStkProactiveSessionStateRegistrants.add(localRegistrant);
  }

  public void lgeRegisterForVTFastFrameUpdate(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeVtFastFrameUpdateRegistrant.add(localRegistrant);
  }

  public void lgeRegisterForVTPeerDisconnect(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeVTPeerDisconnectRegistrant.add(localRegistrant);
  }

  public void lgeRegisterForVtCodecNegoResult(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeVtCodecNegoResultRegistrant.add(localRegistrant);
  }

  public void lgeRegisterForWpbxStateChanged(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeWpbxStateChangedRegistrants.add(localRegistrant);
  }

  public void lgeSetOnDeferredSmsReport(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeDeferredSmsReportRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnDun(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeDunRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnEnablingFacilityLock(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeEnablingFacilityLockRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnFactoryTestRequest(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeFactoryTestRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnSmsDCAbortConnect(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeSMSDCAbortConnectRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnSmsDCConnect(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeSMSDCConnectRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnSmsDCDisConnect(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeSMSDCDisConnectRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnSmsMemStatus(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeSmsMemStatusRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnUsimRemovedStatus(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeUsimRemovedStatusRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnUsimStateChanged(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeUsimStateChangedRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeSetOnXcallstat(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeXcallstatRegistrant = new Registrant(paramHandler, paramInt, paramObject);
  }

  public void lgeUnRegisterForVTFastFrameUpdate(Handler paramHandler)
  {
    this.mLgeVtFastFrameUpdateRegistrant.remove(paramHandler);
  }

  public void lgeUnRegisterForVTPeerDisconnect(Handler paramHandler)
  {
    this.mLgeVTPeerDisconnectRegistrant.remove(paramHandler);
  }

  public void lgeUnRegisterForVtCodecNegoResult(Handler paramHandler)
  {
    this.mLgeVtCodecNegoResultRegistrant.remove(paramHandler);
  }

  public void lgeUnSetOnDeferredSmsReport(Handler paramHandler)
  {
    this.mLgeDeferredSmsReportRegistrant.clear();
  }

  public void lgeUnSetOnDun(Handler paramHandler)
  {
    this.mLgeDunRegistrant.clear();
  }

  public void lgeUnSetOnEnablingFacilityLock(Handler paramHandler)
  {
    this.mLgeEnablingFacilityLockRegistrant.clear();
  }

  public void lgeUnSetOnFactoryTestRequest(Handler paramHandler)
  {
    this.mLgeFactoryTestRegistrant.clear();
  }

  public void lgeUnSetOnSmsDCAbortConnect(Handler paramHandler)
  {
    this.mLgeSMSDCAbortConnectRegistrant.clear();
  }

  public void lgeUnSetOnSmsDCConnect(Handler paramHandler)
  {
    this.mLgeSMSDCConnectRegistrant.clear();
  }

  public void lgeUnSetOnSmsDCDisConnect(Handler paramHandler)
  {
    this.mLgeSMSDCDisConnectRegistrant.clear();
  }

  public void lgeUnSetOnSmsMemStatus(Handler paramHandler)
  {
    this.mLgeSmsMemStatusRegistrant.clear();
  }

  public void lgeUnSetOnUsimRemovedStatus(Handler paramHandler)
  {
    this.mLgeUsimRemovedStatusRegistrant.clear();
  }

  public void lgeUnSetOnUsimStateChanged(Handler paramHandler)
  {
    this.mLgeUsimStateChangedRegistrant.clear();
  }

  public void lgeUnSetOnXcallstat(Handler paramHandler)
  {
    this.mLgeXcallstatRegistrant.clear();
  }

  public void lgeUnregisterFor1XRegDone(Handler paramHandler)
  {
    this.mLge1XRegDoneRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForAocCCMmaxReport(Handler paramHandler)
  {
    this.mLgeAocCCMmaxReportRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForCNAP(Handler paramHandler)
  {
    this.mLgecnapRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForCOLP(Handler paramHandler)
  {
    this.mLgeCOLPRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForCallControlInfo(Handler paramHandler)
  {
    this.mLgeCallControlInfoRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForCdmaSidChangedRegistrants(Handler paramHandler)
  {
    this.mLgeCdmaSidChangedRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForCurrentCallMeter(Handler paramHandler)
  {
    this.mLgeCurrentCallMeterRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForDormancyStatusChanged(Handler paramHandler)
  {
    this.mLgeDormancyRegistrant.remove(paramHandler);
  }

  public void lgeUnregisterForDunBlocked(Handler paramHandler)
  {
    this.mLgeDunBlockedRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForEngineeringMode(Handler paramHandler)
  {
    this.mLgeEngineeringModeRegistrant.remove(paramHandler);
  }

  public void lgeUnregisterForHdrSmpsChanged(Handler paramHandler)
  {
    this.mLgeHdrSmpsChangedRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForLockStateChanged(Handler paramHandler)
  {
    this.mLgeLockStateRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForMtPsPageInd(Handler paramHandler)
  {
    this.mLgeMtPsPageIndRegistrant.remove(paramHandler);
  }

  public void lgeUnregisterForNetworkErrorDisp(Handler paramHandler)
  {
    this.mLgeNetworkErrorDispRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForNetworkRegistrationError()
  {
    this.mLgeNetworkErrorRegistrant.clear();
  }

  public void lgeUnregisterForOtaSessionFail(Handler paramHandler)
  {
    this.mLgeOtaSessionFailRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForOtaSessionSuccess(Handler paramHandler)
  {
    this.mLgeOtaSessionSuccessRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForPbReady(Handler paramHandler)
  {
    this.mLgePbReadyRegistrant.clear();
  }

  public void lgeUnregisterForStkSessionState(Handler paramHandler)
  {
    this.mLgeStkProactiveSessionStateRegistrants.remove(paramHandler);
  }

  public void lgeUnregisterForWpbxStateChanged(Handler paramHandler)
  {
    this.mLgeWpbxStateChangedRegistrants.remove(paramHandler);
  }

  public void lgeregisterForEvdoQoSChanged(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeEvdoQoSChangedRegistrants.add(localRegistrant);
  }

  public void lgeunregisterForEvdoQoSChanged(Handler paramHandler)
  {
    this.mLgeEvdoQoSChangedRegistrants.remove(paramHandler);
  }

  public void registerForCdmaFwdBurstDtmf(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeCdmaFwdBurstDtmfRegistrants.addUnique(paramHandler, paramInt, paramObject);
  }

  public void registerForCdmaFwdContDtmfStart(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeCdmaFwdContDtmfStartRegistrants.addUnique(paramHandler, paramInt, paramObject);
  }

  public void registerForCdmaFwdContDtmfStop(Handler paramHandler, int paramInt, Object paramObject)
  {
    this.mLgeCdmaFwdContDtmfStopRegistrants.addUnique(paramHandler, paramInt, paramObject);
  }

  public void registerForLgeDslgIfaceAddrChangedEv(Handler paramHandler, int paramInt, Object paramObject)
  {
    Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    this.mLgeDslgIfaceAddrChangedEv.add(localRegistrant);
  }

  public void registerForSIMLockedOrAbsent(Handler paramHandler, int paramInt, Object paramObject)
  {
    // XXX
    /*Registrant localRegistrant = new Registrant(paramHandler, paramInt, paramObject);
    synchronized (this.mStateMonitor)
    {
      this.mSIMLockedRegistrants.add(localRegistrant);
      if (this.mState == CommandsInterface.RadioState.SIM_LOCKED_OR_ABSENT)
        localRegistrant.notifyRegistrant(new AsyncResult(null, null, null));
      return;
    }*/
  }

  public void unregisterForCdmaFwdBurstDtmf(Handler paramHandler)
  {
    this.mLgeCdmaFwdBurstDtmfRegistrants.remove(paramHandler);
  }

  public void unregisterForCdmaFwdContDtmfStart(Handler paramHandler)
  {
    this.mLgeCdmaFwdContDtmfStartRegistrants.remove(paramHandler);
  }

  public void unregisterForCdmaFwdContDtmfStop(Handler paramHandler)
  {
    this.mLgeCdmaFwdContDtmfStopRegistrants.remove(paramHandler);
  }

  public void unregisterForLgeDslgIfaceAddrChangedEv(Handler paramHandler)
  {
    this.mLgeDslgIfaceAddrChangedEv.remove(paramHandler);
  }

  public void unregisterForSIMLockedOrAbsent(Handler paramHandler)
  {
    // XXX
    /*synchronized (this.mStateMonitor)
    {
      this.mSIMLockedRegistrants.remove(paramHandler);
      return;
    }*/
  }
}