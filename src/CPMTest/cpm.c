#include <PalmOS.h>
#include <CPMLib.h>

#include "cpm.h"
#include "resource.h"

#define MAX_PROVIDERS 5
#define MAX_KEY 32
#define MAX_TEXT 64

static char providerNames[MAX_PROVIDERS][32];
static char providerOthers[MAX_PROVIDERS][64];
static char providerIDsStr[MAX_PROVIDERS][8];
static char *providerIDsList[MAX_PROVIDERS];
static UInt16 refnum;
static UInt16 numProviders;
static UInt16 providerIndex;
static UInt32 providerIDs[MAX_PROVIDERS];
static UInt16 hashMethod;
static UInt16 cipherMethod;
static char key[MAX_KEY];
static char text[MAX_TEXT];
static char ascHash[MAX_TEXT];
static char ascEnc[MAX_TEXT];
static APKeyInfoType keyInfo;

void CpmInit(UInt16 r, UInt16 num) {
  refnum = r;
  numProviders = num;
  providerIndex = -1;
  hashMethod = 0;
  cipherMethod = 0;

  MemSet(text, sizeof(text), 0);
  MemSet(ascHash, sizeof(ascHash), 0);
  MemSet(ascEnc, sizeof(ascEnc), 0);

  MemSet(&keyInfo, sizeof(APKeyInfoType), 0);
  keyInfo.keyclass = apKeyClassSymmetric;
  keyInfo.usage = apKeyUsageEncryption;
}

void CpmFinish(void) {
  if (keyInfo.length) CPMLibReleaseKeyInfo(refnum, &keyInfo);
}

static void id2s(UInt32 id, char *buf) {
  buf[0] = (char)((id >> 24) & 0xFF);
  buf[1] = (char)((id >> 16) & 0xFF);
  buf[2] = (char)((id >> 8) & 0xFF);
  buf[3] = (char)(id & 0xFF);
  buf[4] = 0;
}

static void CpmFillProviderInfo(FormType *frm) {
  FieldType *fld;

  fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, nameFld));
  FldDelete(fld, 0, 100);
  FldInsert(fld, providerNames[providerIndex], StrLen(providerNames[providerIndex]));

  fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, otherFld));
  FldDelete(fld, 0, 100);
  FldInsert(fld, providerOthers[providerIndex], StrLen(providerOthers[providerIndex]));
}

static void CpmListProviders(FormType *frm) {
  APProviderInfoType providerInfo;
  ControlType *ctl;
  ListType *lst;
  char buf[16];
  UInt16 i;
  Err err;

  lst = (ListType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, providerList));
  ctl = (ControlType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, providerCtl));

  MemSet(providerIDs, sizeof(providerIDs), 0);
  MemSet(providerNames, sizeof(providerNames), 0);
  MemSet(providerOthers, sizeof(providerOthers), 0);
  MemSet(providerIDsList, sizeof(providerIDsList), 0);

  if ((err = CPMLibEnumerateProviders(refnum, providerIDs, &numProviders)) == errNone) {
    for (i = 0; i < numProviders && i < MAX_PROVIDERS; i++) {
      if ((err = CPMLibGetProviderInfo(refnum, providerIDs[i], &providerInfo)) == errNone) {
        id2s(providerIDs[i], providerIDsStr[i]);
        providerIDsList[i] = providerIDsStr[i];
        StrNCopy(providerNames[i], providerInfo.name, 32);
        StrNCopy(providerOthers[i], providerInfo.other, 64);
        //if ((providerInfo.flags & APF_CIPHER)) {
        //}
      } else {
        StrPrintF(buf, "%04X", err);
        FrmCustomAlert(ErrorAlert, "CPMLibGetProviderInfo failed", buf, "");
        numProviders = 0;
      }
    }
  } else {
    StrPrintF(buf, "%04X", err);
    FrmCustomAlert(ErrorAlert, "CPMLibEnumerateProviders failed", buf, "");
    numProviders = 0;
  }

  LstSetHeight(lst, numProviders);
  if (numProviders) {
    if (providerIndex < 0 || providerIndex >= numProviders) providerIndex = 0;
    LstSetListChoices(lst, providerIDsList, numProviders);
    LstSetSelection(lst, providerIndex);
    CtlSetLabel(ctl, providerIDsList[providerIndex]);
    CpmFillProviderInfo(frm);
  } else {
    providerIndex = -1;
    StrNCopy(providerIDsStr[0], "none", 8);
    providerIDsList[0] = providerIDsStr[0];
    LstSetListChoices(lst, providerIDsList, 0);
    CtlSetLabel(ctl, "none");
  }
}

Boolean ProvidersFormHandleEvent(EventType *event) {
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      CpmListProviders(frm);
      FrmDrawForm(frm);
      handled = true;
      break;
    case popSelectEvent:
      frm = FrmGetActiveForm();
      providerIndex = event->data.popSelect.selection;
      CpmFillProviderInfo(frm);
      break;
    case menuEvent:
      FrmGotoForm(event->data.menu.itemID);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static char bin2hex1(UInt8 b) {
  if (b < 10) return b + '0';
  return b - 10 + 'A';
}

static void bin2hex(UInt8 b, char *h) {
  h[0] = bin2hex1(b >> 4);
  h[1] = bin2hex1(b & 0xF);
  h[2] = 0;
}

static void CpmHash(UInt32 providerID, UInt16 hashMethod) {
  APHashInfoType hashInfo;
  UInt8 binHash[64];
  UInt16 method;
  UInt32 i, len;
  char buf[16];
  Err err;

  switch (hashMethod) {
    case 0: method = apHashTypeMD5;  break;
    case 1: method = apHashTypeSHA1; break;
    default: method = apHashTypeSHA1; break;
  }

  MemSet(&binHash, sizeof(binHash), 0);
  MemSet(&ascHash, sizeof(ascHash), 0);

  MemSet(&hashInfo, sizeof(APHashInfoType), 0);
  hashInfo.providerContext.providerID = providerID;

  if ((err = CPMLibHash(refnum, method, &hashInfo, (UInt8 *)text, StrLen(text), binHash, &len)) == errNone) {
    for (i = 0; i < len; i++) {
      bin2hex(binHash[i], buf);
      StrNCat(ascHash, buf, sizeof(ascHash) - i*2);
    }
  } else {
    StrPrintF(buf, "%04X", err);
    FrmCustomAlert(ErrorAlert, "CPMLibHash failed", buf, "");
  }
}

static UInt8 hex2bin1(char h) {
  if (h >= '0' && h <= '9') return h - '0';
  if (h >= 'A' && h <= 'F') return h - 'A' + 10;
  if (h >= 'a' && h <= 'f') return h - 'a' + 10;
  return 0;
}

static UInt8 hex2bin(char *h) {
  return hex2bin1(h[0])* 16 + hex2bin1(h[1]);
}

static void CpmCipher(UInt32 providerID, UInt16 cipherMethod, Boolean encrypt) {
  APCipherInfoType cipherInfo;
  UInt8 encrypted[MAX_TEXT];
  UInt16 method, keylen;
  UInt32 i, len;
  char buf[16];
  Err err;

  switch (cipherMethod) {
    case 0: method = apSymmetricType3DES_EDE3; keylen = 24; break;
    case 1: method = apSymmetricTypeRC4;       keylen = 16; break;
    case 2: method = apSymmetricTypeRijndael;  keylen = 16; break;
    case 3: method = apSymmetricTypeRijndael;  keylen = 32; break;
    default: method = apSymmetricTypeRijndael; keylen = 32; break;
  }

  if (keyInfo.providerContext.providerID != providerID || keylen != keyInfo.length) {
    if (keyInfo.length) CPMLibReleaseKeyInfo(refnum, &keyInfo);
    keyInfo.providerContext.providerID = providerID;
    keyInfo.length = keylen;
    if ((err = CPMLibGenerateKey(refnum, NULL, 0, &keyInfo)) != errNone) {
      StrPrintF(buf, "%04X", err);
      FrmCustomAlert(ErrorAlert, "CPMLibGenerateKey failed", buf, "");
    }
  } else {
    err = errNone;
  }

  if (err == errNone) {
    MemSet(&cipherInfo, sizeof(APCipherInfoType), 0);
    cipherInfo.providerContext.providerID = providerID;
    cipherInfo.type = method;

    if (encrypt) {
      MemSet(&ascEnc, sizeof(ascEnc), 0);
      len = MAX_TEXT;
      if ((err = CPMLibEncrypt(refnum, &keyInfo, &cipherInfo, (UInt8 *)text, MAX_TEXT, encrypted, &len)) == errNone) {
        for (i = 0; i < len; i++) {
          bin2hex(encrypted[i], buf);
          StrNCat(ascEnc, buf, sizeof(ascEnc) - i*2);
        }
      } else {
        StrPrintF(buf, "%04X", err);
        FrmCustomAlert(ErrorAlert, "CPMLibEncrypt failed", buf, "");
      }
    } else {
      MemSet(&text, sizeof(text), 0);
      MemSet(&encrypted, sizeof(encrypted), 0);
      for (i = 0; i < MAX_TEXT; i++) {
        encrypted[i] = hex2bin(&ascEnc[i*2]);
      }
      len = MAX_TEXT;
      if ((err = CPMLibDecrypt(refnum, &keyInfo, &cipherInfo, encrypted, MAX_TEXT, (UInt8 *)text, &len)) != errNone) {
        StrPrintF(buf, "%04X", err);
        FrmCustomAlert(ErrorAlert, "CPMLibDecrypt failed", buf, "");
      }
    }
  }
}

Boolean CpmFormHandleEvent(EventType *event) {
  FormType *frm;
  ListType *lst;
  FieldType *fld;
  ControlType *ctl;
  UInt16 formId;
  char *s;
  Boolean handled = false;

  formId = FrmGetActiveFormID();

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();

      lst = (ListType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, methodList));
      LstSetSelection(lst, formId == HashForm ? hashMethod : cipherMethod);

      ctl = (ControlType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, methodCtl));
      CtlSetLabel(ctl, LstGetSelectionText(lst, LstGetSelection(lst)));

      if (formId == EncryptForm || formId == DecryptForm) {
        fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, keyFld));
        FldDelete(fld, 0, MAX_TEXT);
        if (key[0]) FldInsert(fld, key, StrLen(key));
      }

      switch (formId) {
        case HashForm:
          fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, textFld));
          FldDelete(fld, 0, MAX_TEXT);
          if (text[0]) FldInsert(fld, text, StrLen(text));
          fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, binFld));
          FldDelete(fld, 0, MAX_TEXT);
          if (ascHash[0]) FldInsert(fld, ascHash, StrLen(ascHash));
          break;
        case EncryptForm:
        case DecryptForm:
          fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, textFld));
          FldDelete(fld, 0, MAX_TEXT);
          if (text[0]) FldInsert(fld, text, StrLen(text));
          fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, binFld));
          FldDelete(fld, 0, MAX_TEXT);
          if (ascEnc[0]) FldInsert(fld, ascEnc, StrLen(ascEnc));
          break;
      }

      FrmDrawForm(frm);
      handled = true;
      break;
    case popSelectEvent:
      frm = FrmGetActiveForm();
      lst = (ListType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, methodList));
      if (formId == HashForm) {
        hashMethod = LstGetSelection(lst);
      } else {
        cipherMethod = LstGetSelection(lst);
      }
      break;
    case ctlSelectEvent:
      if (event->data.ctlSelect.controlID == goCtl && providerIndex != -1) {
        frm = FrmGetActiveForm();
        lst = (ListType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, methodList));

        switch (formId) {
          case HashForm:
            MemSet(&text, sizeof(text), 0);
            fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, textFld));
            s = FldGetTextPtr(fld);
            if (s && s[0]) {
              StrNCopy(text, s, MAX_TEXT);
              hashMethod = LstGetSelection(lst);
              CpmHash(providerIDs[providerIndex], hashMethod);
              fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, binFld));
              FldDelete(fld, 0, MAX_TEXT);
              FldInsert(fld, ascHash, StrLen(ascHash));
            }
            break;
          case EncryptForm:
            MemSet(&text, sizeof(text), 0);
            fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, textFld));
            s = FldGetTextPtr(fld);
            if (s && s[0]) {
              StrNCopy(text, s, MAX_TEXT);
              cipherMethod = LstGetSelection(lst);
              CpmCipher(providerIDs[providerIndex], cipherMethod, true);
              fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, binFld));
              FldDelete(fld, 0, MAX_TEXT);
              FldInsert(fld, ascEnc, StrLen(ascEnc));
            }
            break;
          case DecryptForm:
            MemSet(ascEnc, sizeof(ascEnc), 0);
            fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, binFld));
            s = FldGetTextPtr(fld);
            if (s && s[0]) {
              StrNCopy(ascEnc, s, MAX_TEXT);
              cipherMethod = LstGetSelection(lst);
              CpmCipher(providerIDs[providerIndex], cipherMethod, false);
              fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, textFld));
              FldDelete(fld, 0, MAX_TEXT);
              FldInsert(fld, text, StrLen(text));
            }
            break;
        }
        handled = true;
      }
      break;
    case menuEvent:
      FrmGotoForm(event->data.menu.itemID);
      handled = true;
      break;
    case frmCloseEvent:
      frm = FrmGetActiveForm();

      MemSet(text, sizeof(text), 0);
      fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, textFld));
      s = FldGetTextPtr(fld);
      if (s && s[0]) StrNCopy(text, s, MAX_TEXT);

      fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, binFld));
      s = FldGetTextPtr(fld);

      switch (formId) {
        case HashForm:
          MemSet(ascHash, sizeof(ascHash), 0);
          if (s && s[0]) StrNCopy(ascHash, s, MAX_TEXT);
          break;
        case EncryptForm:
        case DecryptForm:
          MemSet(ascEnc, sizeof(ascEnc), 0);
          if (s && s[0]) StrNCopy(ascEnc, s, MAX_TEXT);
          break;
      }
      break;
    default:
      break;
  }

  return handled;
}
