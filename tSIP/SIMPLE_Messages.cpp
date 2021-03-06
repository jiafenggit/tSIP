//---------------------------------------------------------------------------


#pragma hdrstop

#include "SIMPLE_Messages.h"
#include "FormMessage.h"
#include "Log.h"
#include "Settings.h"
#include <set>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace SIMPLE_Messages;

namespace
{
	std::set<TfrmMessage*> messageWindows;
}	// namespace

namespace SIMPLE_Messages
{

void RegisterWindow(TfrmMessage *frmMessage)
{
	messageWindows.insert(frmMessage);
}

void UnregisterWindow(TfrmMessage *frmMessage)
{
	messageWindows.erase(frmMessage);
}

void CloseAllWindows(void)
{
	std::set<TfrmMessage*>::iterator iter;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); )
	{
		TfrmMessage *frm = *(iter++);
		frm->Close();
	}
}

bool IsTargetMatching(AnsiString remote, AnsiString target)
{
	if (remote == target)
	{
		return true;
	}

	if (target.Pos("sip:") != 1)
	{
		if (appSettings.uaConf.accounts.size() > 0)
		{
			target = (AnsiString)"sip:" + target + "@" + appSettings.uaConf.accounts[0].reg_server.c_str();
			if (remote == target)
			{
            	return true;
			}
		}
	}

	return false;
}

void OnIncomingMessage(AnsiString caller, AnsiString contentType, AnsiString body)
{
	LOG("Received message from %s: ContentType %s, body [%s]\n", caller.c_str(), contentType.c_str(), body.c_str());
	std::set<TfrmMessage*>::iterator iter;
	TfrmMessage *frm = NULL;
	int incomingCount = 0;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); ++iter)
	{
		TfrmMessage *frmIter = *(iter);
		if (frmIter->GetIncoming())
		{
			incomingCount++;
		}
		if (IsTargetMatching(caller, frmIter->GetTarget()))
		{
			frm = frmIter;
			break;
		}
	}
	if (frm == NULL && incomingCount > 25)
	{
		LOG("Limit for MESSAGE windows for incoming transactions exceeded, ignoring incoming message\n");
		return;
	}
	if (frm == NULL)
	{
		frm = new TfrmMessage(NULL);
		frm->SetIncoming();
		frm->SetTarget(caller);
	}
	frm->AddIncomingMessage(contentType, body);
	frm->Show();
}

void OnMessageStatus(int requestUid, int requestError, int sipCode)
{
	LOG("MessageStatus: requestUid = %d, requestError = %d, sipCode = %d", requestUid, requestError, sipCode);
	std::set<TfrmMessage*>::iterator iter;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); ++iter)
	{
		TfrmMessage *frmIter = *(iter);
		if (frmIter->HandleMessageStatus(requestUid, requestError, sipCode) == 0)
		{
			return;
		}
	}
}

}

