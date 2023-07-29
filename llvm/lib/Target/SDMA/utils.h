#pragma once

#define not_implemented()                                                      \
  report_fatal_error(                                                          \
      llvm::createStringError(std::errc::operation_not_permitted, "todo: (line %d) in %s",  \
                            __LINE__, \
                              (const char *)__PRETTY_FUNCTION__))
