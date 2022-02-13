#include <string.h>
#include <stdarg.h>

#include <wist/str.h>
#include <wist/mem.h>

WistStr
wist_str_null_terminate(WistStrRef ref)
{
    char *buf = WIST_NEW_ARR(uint8_t, ref.len + 1);
    memcpy(buf, ref.str, ref.len);
    buf[ref.len] = '\0';

    WistStr str = {
        .str = buf,
        .len = ref.len + 1,
    };

    return str;
}

WistStrRef
wist_str_from_c(const char *c_str)
{
    WistStrRef ref = {
        .str = (const uint8_t *) c_str,
        .len = strlen(c_str) + 1,
    };
    return ref;
}

void
wist_str_libc_free(WistStr str)
{
    WIST_FREE((uint8_t *)str.str);
}

WistStr
wist_str_clone(WistStrRef ref)
{
    uint8_t *buf = WIST_NEW_ARR(uint8_t, ref.len);
    memcpy(buf, ref.str, ref.len);

    WistStr ret = {
        .str = buf,
        .len = ref.len,
    };

    return ret;
}

WistStrRef
wist_str_to_ref(WistStr str)
{
    WistStrRef ref = {
        .str = str.str,
        .len = str.len,
    };

    return ref;
}

uint32_t
wist_str_hash(WistStrRef ref)
{
    return ref.len;
}

WistStr
wist_str_from_slice(const uint8_t *src,
                    size_t len)
{
    WistStr str;
    uint8_t *buf = WIST_NEW_ARR(uint8_t, len);
    memcpy(buf, src, len);
    str.str = buf;
    str.len = len;
    return str;
}

WistStr
wist_format(const char *str, ...)
{
    va_list l1, l2;
    va_start(l1, str);
    va_copy(l2, l1);
    
    size_t len = strlen(str);
    size_t bytes_needed = len;
    
    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == '%')
        {
            i++;
            switch (str[i])
            {
                case '%':
                    bytes_needed -= 1;
                    break;
                case 's': {

                    WistStrRef *ref = va_arg(l1, WistStrRef *);
                    bytes_needed += ref->len;
                    break;
                }
                    
                case 'c':
                    bytes_needed -= 1;
                    break;
            }
        }
    }
    
    uint8_t *buf = WIST_NEW_ARR(uint8_t, bytes_needed);
    size_t pos = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == '%')
        {
            i++;
            switch (str[i])
            {
                case '%':
                    buf[pos++] = '%';
                    break;
                case 's': {
                    WistStrRef *ref = va_arg(l2, WistStrRef *);
                    buf[pos++] = '\'';
                    if (strncmp(ref->str, "\n", 1) == 0)
                    {
                        buf[pos++] = ' ';
                    }
                    else
                    {
                        for (size_t i = 0; i < ref->len; i++)
                        {
                            buf[pos++] = ref->str[i];
                        }
                    }
                    buf[pos++] = '\'';
                    break;
                }
                    
                
                case 'c': {
                    buf[pos++] = va_arg(l2, uint8_t);
                    break;
                }
            }
        }
        else
        {
            buf[pos++] = str[i];
        }
    }
    
    WistStr ret = {
        .str = buf,
        .len = bytes_needed,
    };    
    
    return ret;
}
