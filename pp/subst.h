#pragma once

/**
 * Perform all macro substitutions on a given string
 *
 * @param s string to substitute
 * @param[optional] defs defines to substitute against
 * @return resulting string (free with free())
 */
extern char *subst_tokens(const char *s, const defines *defs);
