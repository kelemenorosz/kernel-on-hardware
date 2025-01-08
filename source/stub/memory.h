#ifndef MEMORY_H
#define MEMORY_H

void memset(void* ptr, uint8_t value, size_t num);
void memcpy(void* destination, const void* source, size_t num);

#endif /* MEMORY_H */