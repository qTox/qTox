#ifndef __STATUS_NOTIFIER_ENUMS_H__
#define __STATUS_NOTIFIER_ENUMS_H__
#include "statusnotifier.h"



GType status_notifier_error_get_type (void);
#define TYPE_STATUS_NOTIFIER_ERROR (status_notifier_error_get_type())
GType status_notifier_state_get_type (void);
#define TYPE_STATUS_NOTIFIER_STATE (status_notifier_state_get_type())
GType status_notifier_icon_get_type (void);
#define TYPE_STATUS_NOTIFIER_ICON (status_notifier_icon_get_type())
GType status_notifier_category_get_type (void);
#define TYPE_STATUS_NOTIFIER_CATEGORY (status_notifier_category_get_type())
GType status_notifier_status_get_type (void);
#define TYPE_STATUS_NOTIFIER_STATUS (status_notifier_status_get_type())
GType status_notifier_scroll_orientation_get_type (void);
#define TYPE_STATUS_NOTIFIER_SCROLL_ORIENTATION (status_notifier_scroll_orientation_get_type())
G_END_DECLS

#endif /* __STATUS_NOTIFIER_ENUMS_H__ */



