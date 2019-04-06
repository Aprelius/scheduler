#pragma once

namespace Scheduler {

    static const char* hexChars = "0123456789abcdef";

    inline bool IsDigit(char c) { return c >= 48 && c <= 57; }

    inline bool IsHex(char c)
    {
        return (c >= 97 && c <= 102) || (c >= 65 && c <= 70)
            || (c >= 48 && c <= 57);
    }

    inline bool IsSpace(char c) { return (c == ' ' || c == '\t' || c == '\n' || c == '\r'); }

    inline bool IsLower(char c) { return c >= 97 && c <= 122; }

	inline bool IsUpper(char c) { return c >= 65 && c <= 90; }

    inline unsigned char ToNibble(char c)
	{
		if (IsDigit(c)) return c - '0';
		if (IsLower(c)) return c - 'a' + 10;
		if (IsUpper(c)) return c - 'A' + 10;
		return 255;
	}

}  // namespace Scheduler
