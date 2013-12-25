/*
 * Copyright 2013-2013 Marcel Mueller
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define  INCL_PM
#define  INCL_BASE
#define  INCL_GPI

#include "Frontend.h"
#include "Calibrate.h"

#include <cpp/url123.h>


/*double Frontend::CalibratePage::XtractDeltaGain(const DataRowType& row, void* col)
{ return log(row[3] / row[1]) * (20. / M_LN10);
}

double Frontend::CalibratePage::XtractDeltaDelay(const DataRowType& row, void* col)
{ return (row[4] - row[2]) * row[0];
}*/

Frontend::CalibratePage::CalibratePage(Frontend& parent)
: OpenLoopPage(parent, DLG_CALIBRATE, Calibrate::VTable)
{ MajorTitle = "~Calibrate";
  MinorTitle = "Calibrate sound card";

  Response.AddGraph("< L gain", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::LGain, ResponseGraph::GF_None, CLR_BLUE);
  Response.AddGraph("< R gain", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::RGain, ResponseGraph::GF_None, CLR_RED);
  Response.AddGraph("L ph.del. >", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractPhaseDelay, (void*)Calibrate::CalibrationFile::LDelay, ResponseGraph::GF_Y2, CLR_GREEN);
  Response.AddGraph("R ph.del. >", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractPhaseDelay, (void*)Calibrate::CalibrationFile::RDelay, ResponseGraph::GF_Y2, CLR_PINK);
  Response.AddGraph("<  gain", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::DeltaGain, ResponseGraph::GF_None, CLR_CYAN);
  Response.AddGraph(" ph.del. >", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractPhaseDelay, (void*)Calibrate::CalibrationFile::DeltaDelay, ResponseGraph::GF_Y2, CLR_YELLOW);
  XTalk.AddGraph("< R2L", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::R2LGain, ResponseGraph::GF_None, CLR_BLUE);
  XTalk.AddGraph("< L2R", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::L2RGain, ResponseGraph::GF_None, CLR_RED);
  XTalk.AddGraph("< L IM2", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::LIntermod, ResponseGraph::GF_None, CLR_CYAN);
  XTalk.AddGraph("< R IM2", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractGain, (void*)Calibrate::CalibrationFile::RIntermod, ResponseGraph::GF_None, CLR_YELLOW);
  //XTalk.AddGraph("R2L del. t >", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractDelay, (void*)6, ResponseGraph::GF_Y2, CLR_GREEN);
  //XTalk.AddGraph("L2R del. t >", Calibrate::GetData(), &Frontend::XtractFrequency, &Frontend::XtractDelay, (void*)8, ResponseGraph::GF_Y2, CLR_PINK);
}

Frontend::CalibratePage::~CalibratePage()
{}

MRESULT Frontend::CalibratePage::DlgProc(ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg)
  {case WM_INITDLG:
    Response.Attach(GetCtrl(CC_RESULT));
    XTalk.Attach(GetCtrl(CC_RESULT2));
    break;

   case WM_CONTROL:
    switch (SHORT1FROMMP(mp1))
    {case RB_CH_BOTH:
      if (SHORT2FROMMP(mp1) == BN_CLICKED && RadioButton(HWNDFROMMP(mp2)).CheckState())
      { CheckBox diffout(+GetCtrl(CB_DIFFOUT));
        diffout.CheckState(false);
        diffout.Enabled(false);
      }
      break;
     case RB_CH_LEFT:
     case RB_CH_RIGHT:
      if (SHORT2FROMMP(mp1) == BN_CLICKED && !RadioButton(GetCtrl(RB_CH_BOTH)).CheckState())
        CheckBox(+GetCtrl(CB_DIFFOUT)).Enabled(true);
      break;
     case CB_CAL_FILE:
      if (SHORT2FROMMP(mp1) == CBN_ENTER)
        // TODO
        ;
      break;
    }
    break;
  }

  return OpenLoopPage::DlgProc(msg, mp1, mp2);
}

void Frontend::CalibratePage::LoadControlValues(const Calibrate::CalibrationFile& data)
{
  RadioButton(+GetCtrl(RB_STEREO_LOOP + data.Mode)).CheckState(true);

  OpenLoopPage::LoadControlValues(data);

  Response.SetAxes(ResponseGraph::AF_LogX, data.RefFMin, data.RefFMax,
    data.GainLow, data.GainHigh, data.DelayLow, data.DelayHigh);
  XTalk.SetAxes(ResponseGraph::AF_LogX, data.RefFMin, data.RefFMax,
    data.Gain2Low, data.Gain2High, NAN,NAN);
}
void Frontend::CalibratePage::LoadControlValues()
{ SyncAccess<Calibrate::CalibrationFile> data(Calibrate::GetData());
  LoadControlValues(*data);
}

void Frontend::CalibratePage::StoreControlValues(Calibrate::CalibrationFile& data)
{
  data.Mode = (Calibrate::MeasureMode)(RadioButton(+GetCtrl(RB_STEREO_LOOP)).CheckID() - RB_STEREO_LOOP);

  OpenLoopPage::StoreControlValues(data);

  if (data.FileName)
    data.FileName = data.FileName + ".calibrate";
}
void Frontend::CalibratePage::StoreControlValues()
{ SyncAccess<Calibrate::CalibrationFile> data(Calibrate::GetData());
  StoreControlValues(*data);
}

void Frontend::CalibratePage::SetRunning(bool running)
{
  RadioButton(+GetCtrl(RB_STEREO_LOOP)).EnableAll(!running);
  OpenLoopPage::SetRunning(running);
}

void Frontend::CalibratePage::InvalidateGraph()
{ Response.Invalidate();
  XTalk.Invalidate();
}

LONG Frontend::CalibratePage::DoLoadFile(FILEDLG& fdlg)
{
  fdlg.pszTitle = "Load DRC123 calibration file";
  // PM crashes if type is not writable
  char type[_MAX_PATH] = "DRC123 Calibration File (*.calibrate)";
  fdlg.pszIType = type;
  LONG ret = OpenLoopPage::DoLoadFile(fdlg);
  if (ret == DID_OK)
  { { SyncAccess<Calibrate::CalibrationFile> data(Calibrate::GetData());
      if (!data->Load(fdlg.szFullFile))
        return false;
      LoadControlValues(*data);
    }
  }
  InvalidateGraph();
  return ret;
}

xstring Frontend::CalibratePage::DoSaveFile()
{ SyncAccess<Calibrate::CalibrationFile> data(Calibrate::GetData());
  if (data->Save(data->FileName))
    return xstring();
  return data->FileName;
}


/*MRESULT Frontend::CalibrateExtPage::DlgProc(ULONG msg, MPARAM mp1, MPARAM mp2)
{
  return PageBase::DlgProc(msg, mp1, mp2);
}*/

void Frontend::CalibrateExtPage::LoadControlValues(const Calibrate::CalibrationFile& data)
{
  ExtPage::LoadControlValues(data);

  SetValue(GetCtrl(EF_GAIN2_LOW), data.Gain2Low);
  SetValue(GetCtrl(EF_GAIN2_HIGH), data.Gain2High);
}
void Frontend::CalibrateExtPage::LoadControlValues()
{ SyncAccess<Calibrate::CalibrationFile> data(Calibrate::GetData());
  LoadControlValues(*data);
}

void Frontend::CalibrateExtPage::LoadDefaultValues()
{ LoadControlValues(Calibrate::DefData);
}

void Frontend::CalibrateExtPage::StoreControlValues(Calibrate::CalibrationFile& data)
{
  ExtPage::StoreControlValues(data);

  GetValue(GetCtrl(EF_GAIN2_LOW), data.Gain2Low);
  GetValue(GetCtrl(EF_GAIN2_HIGH), data.Gain2High);
}
void Frontend::CalibrateExtPage::StoreControlValues()
{ SyncAccess<Calibrate::CalibrationFile> data(Calibrate::GetData());
  StoreControlValues(*data);
}
