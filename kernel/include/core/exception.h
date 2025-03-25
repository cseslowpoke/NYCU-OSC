#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#include "common/types.h"
#include "common/utils.h"

void default_exception_handler();

void _el1_lower_el_aarch64_sync_handler();

void _el1_lower_el_aarch64_irq_handler();

void _el1_lower_el_aarch64_fiq_handler();

void _el1_lower_el_aarch64_serror_handler();

void _el1_current_el_aarch64_sync_handler();

void _el1_current_el_aarch64_irq_handler();

void _el1_current_el_aarch64_fiq_handler();

void _el1_current_el_aarch64_serror_handler();

#endif
