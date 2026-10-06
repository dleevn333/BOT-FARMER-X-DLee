#pragma once
struct FarmNoticeRetry {FarmNoticeKind kind;bool closed;};
inline thread_local ThongTinTool* farmNoticeOwner=nullptr;
inline thread_local bool farmNoticeRecovering=false;
struct FarmNoticeScope {
    ThongTinTool* previous;
    explicit FarmNoticeScope(ThongTinTool* owner):previous(farmNoticeOwner){farmNoticeOwner=owner;}
    ~FarmNoticeScope(){farmNoticeOwner=previous;}
};
cv::Mat FarmRecoverNotice(HWND game,const cv::Mat& frame);
