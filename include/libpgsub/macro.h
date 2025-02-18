#pragma once

#ifdef CONFIG_WITH_NAMESPACE

#define PGSUB_NAMESPACE_BEGIN namespace LibPGSub {
#define PGSUB_NAMESPACE_END }

#else
#define PGSUB_NAMESPACE_BEGIN
#define PGSUB_NAMESPACE_END
#endif