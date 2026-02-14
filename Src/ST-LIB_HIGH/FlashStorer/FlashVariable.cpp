#include "FlashStorer/FlashVariable.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

void* FlashVariable::get_pointer() { return FlashVariable::v_pointer; }

size_t FlashVariable::get_size() { return FlashVariable::size; }
