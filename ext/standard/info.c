/* 
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/head.h"
#include "info.h"
#include "SAPI.h"
#if !defined(PHP_WIN32)
#include "build-defs.h"
#endif
#include "zend_globals.h"		/* needs ELS */


#define PHP3_CONF_LONG(directive,value1,value2) \
	php_printf("<tr><td bgcolor=\"" PHP_ENTRY_NAME_COLOR "\">%s</td><td bgcolor=\"" PHP_CONTENTS_COLOR "\">%ld</td><td bgcolor=\"" PHP_CONTENTS_COLOR "\">%ld</td></tr>\n",directive,value1,value2);

#define SECTION(name)  PUTS("<hr><h2>" name "</h2>\n")

#define CREDIT_LINE(module, authors) php_info_print_table_row(2, module, authors)


static int _display_module_info(zend_module_entry *module, void *arg)
{
	int show_info_func = *((int *) arg);

	if (show_info_func && module->info_func) {
		php_printf("<hr><a name=\"module_%s\"><h2>%s</h2>\n", module->name, module->name);
		module->info_func(module);
	} else if (!show_info_func && !module->info_func) {
		php_printf("<tr><td bgcolor=\"" PHP_CONTENTS_COLOR "\">");
		php_printf(module->name);
		php_printf("</td><tr>\n");
	}
	return 0;
}


static void php_print_gpcse_array(char *name, uint name_length ELS_DC)
{
	zval **data, **tmp;
	char *string_key;
	ulong num_key;

	if (zend_hash_find(&EG(symbol_table), name, name_length+1, (void **) &data)!=FAILURE
		&& ((*data)->type==IS_ARRAY)) {
		zend_hash_internal_pointer_reset((*data)->value.ht);
		while (zend_hash_get_current_data((*data)->value.ht, (void **) &tmp) == SUCCESS) {
			zval tmp2, *value_ptr;

			if ((*tmp)->type != IS_STRING) {
				tmp2 = **tmp;
				zval_copy_ctor(&tmp2);
				convert_to_string(&tmp2);
				value_ptr = &tmp2;
			} else {
				value_ptr = *tmp;
			}
			PUTS("<tr><td bgcolor=\"" PHP_ENTRY_NAME_COLOR "\"><b>");
			PUTS(name);
			PUTS("[\"");
			switch (zend_hash_get_current_key((*data)->value.ht, &string_key, &num_key)) {
				case HASH_KEY_IS_STRING:
					PUTS(string_key);
					efree(string_key);
					break;
				case HASH_KEY_IS_LONG:
					php_printf("%ld",num_key);
					break;
			}
			PUTS("\"]</b></td><td bgcolor=\"" PHP_CONTENTS_COLOR "\">");
			if ((*tmp)->type == IS_ARRAY) {
				PUTS("<pre>");
				zend_print_zval_r(*tmp, 0);
				PUTS("</pre>");
			} else {
				PUTS(value_ptr->value.str.val);
			}
			PUTS("</td></tr>\n");
			zend_hash_move_forward((*data)->value.ht);
			if (value_ptr==&tmp2) {
				zval_dtor(value_ptr);
			}
		}
	}
}


PHPAPI void php_print_info(int flag)
{
	char **env,*tmp1,*tmp2;
	char *php_uname;
	int expose_php = INI_INT("expose_php");
#ifdef PHP_WIN32
	char php_windows_uname[256];
	DWORD dwBuild=0;
	DWORD dwVersion = GetVersion();
	DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
#endif
	ELS_FETCH();
	SLS_FETCH();

	
	if (flag & PHP_INFO_GENERAL) {
#ifdef PHP_WIN32
		// Get build numbers for Windows NT or Win95
		if (dwVersion < 0x80000000){
			dwBuild = (DWORD)(HIWORD(dwVersion));
			snprintf(php_windows_uname,255,"%s %d.%d build %d","Windows NT",dwWindowsMajorVersion,dwWindowsMinorVersion,dwBuild);
		} else {
			snprintf(php_windows_uname,255,"%s %d.%d","Windows 95/98",dwWindowsMajorVersion,dwWindowsMinorVersion);
		}
		php_uname = php_windows_uname;
#else
		php_uname=PHP_UNAME;
#endif


		php_printf("<center><h1>PHP Version %s</h1></center>\n", PHP_VERSION);

		PUTS("<hr>");
		if (expose_php) {
			PUTS("<a href=\"http://www.php.net/\"><img src=\"");
			if (SG(request_info).request_uri) {
				PUTS(SG(request_info).request_uri);
			}
			PUTS("?=PHPE9568F34-D428-11d2-A769-00AA001ACF42\" border=\"0\" align=\"right\"></a>\n");
		}
		php_printf("System: %s<br>Build Date: %s\n<br>", php_uname, __DATE__);
#ifdef CONFIGURE_COMMAND
		php_printf("Configure command: %s<br>\n", CONFIGURE_COMMAND);
#endif
		php_printf("php.ini path:  %s<br>\n", CONFIGURATION_FILE_PATH);
		php_printf("ZEND_DEBUG=%d<br>\n", ZEND_DEBUG);
		if (sapi_module.name)
			php_printf("SAPI=%s<br>\n", sapi_module.name);
#ifdef ZTS
		php_printf("ZTS is defined");
#else
		php_printf("ZTS is undefined");
#endif
		/* Zend Engine */
		PUTS("<hr>");
		if (expose_php) {
			PUTS("<a href=\"http://www.zend.com/\"><img src=\"");
			if (SG(request_info).request_uri) {
				PUTS(SG(request_info).request_uri);
			}
			PUTS("?=PHPE9568F35-D428-11d2-A769-00AA001ACF42\" border=\"0\" align=\"right\"></a>\n");
		}
		php_printf("This program makes use of the Zend scripting language engine:<br><pre>%s</pre>", get_zend_version());
	}

	PUTS("<center>");

	if ((flag & PHP_INFO_CREDITS) && expose_php) {	
		PUTS("<hr>");
		PUTS("<a href=\"");
		if (SG(request_info).request_uri) {
			PUTS(SG(request_info).request_uri);
		}
		PUTS("?=PHPB8B5F2A0-3C92-11d3-A3A9-4C7B08C10000\">");
		PUTS("<h1>PHP 4.0 Credits</h1>\n");
		PUTS("</a>\n");
	}


	if (flag & PHP_INFO_CONFIGURATION) {
		PUTS("<hr><h1>Configuration</h1>\n");
		PUTS("<h2>PHP Core</h2>\n");
		display_ini_entries(NULL);
	}

	if (flag & PHP_INFO_MODULES) {
		int show_info_func;

		show_info_func = 1;
		zend_hash_apply_with_argument(&module_registry, (int (*)(void *, void *)) _display_module_info, &show_info_func);

		SECTION("Additional Modules");
		PUTS("<table border=5 width=\"600\">\n");
		show_info_func = 0;
		zend_hash_apply_with_argument(&module_registry, (int (*)(void *, void *)) _display_module_info, &show_info_func);
		PUTS("</table>\n");
	}

	if (flag & PHP_INFO_ENVIRONMENT) {
		SECTION("Environment");
		PUTS("<table border=5 width=\"600\">\n");
		php_info_print_table_header(2, "Variable", "Value");
		for (env=environ; env!=NULL && *env !=NULL; env++) {
			tmp1 = estrdup(*env);
			if (!(tmp2=strchr(tmp1,'='))) { /* malformed entry? */
				efree(tmp1);
				continue;
			}
			*tmp2 = 0;
			tmp2++;
			php_info_print_table_row(2, tmp1, tmp2);
			efree(tmp1);
		}
		PUTS("</table>\n");
	}

	if (flag & PHP_INFO_VARIABLES) {
		pval **data;

		SECTION("PHP Variables");

		PUTS("<table border=5 width=\"600\">\n");
		php_info_print_table_header(2, "Variable", "Value");
		if (zend_hash_find(&EG(symbol_table), "PHP_SELF", sizeof("PHP_SELF"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_SELF", (*data)->value.str.val);
		}
		if (zend_hash_find(&EG(symbol_table), "PHP_AUTH_TYPE", sizeof("PHP_AUTH_TYPE"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_AUTH_TYPE", (*data)->value.str.val);
		}
		if (zend_hash_find(&EG(symbol_table), "PHP_AUTH_USER", sizeof("PHP_AUTH_USER"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_AUTH_USER", (*data)->value.str.val);
		}
		if (zend_hash_find(&EG(symbol_table), "PHP_AUTH_PW", sizeof("PHP_AUTH_PW"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_AUTH_PW", (*data)->value.str.val);
		}
		php_print_gpcse_array("HTTP_GET_VARS", sizeof("HTTP_GET_VARS")-1 ELS_CC);
		php_print_gpcse_array("HTTP_POST_VARS", sizeof("HTTP_POST_VARS")-1 ELS_CC);
		php_print_gpcse_array("HTTP_POST_FILES", sizeof("HTTP_POST_FILES")-1 ELS_CC);
		php_print_gpcse_array("HTTP_COOKIE_VARS", sizeof("HTTP_COOKIE_VARS")-1 ELS_CC);
		php_print_gpcse_array("HTTP_SERVER_VARS", sizeof("HTTP_SERVER_VARS")-1 ELS_CC);
		php_print_gpcse_array("HTTP_ENV_VARS", sizeof("HTTP_ENV_VARS")-1 ELS_CC);
		PUTS("</table>\n");
	}

	PUTS("</center>");


	if (flag & PHP_INFO_LICENSE) {
		SECTION("PHP License");
		PUTS("<PRE>This program is free software; you can redistribute it and/or modify\n");
		PUTS("it under the terms of the PHP License as published by the PHP Group\n");
		PUTS("and included in the distribution in the file:  LICENSE\n");
		PUTS("\n");
		PUTS("This program is distributed in the hope that it will be useful,\n");
		PUTS("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
		PUTS("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
		PUTS("\n");
		PUTS("If you did not receive a copy of the PHP license, or have any questions about\n");
		PUTS("PHP licensing, please contact license@php.net.</PRE>\n");
	}
}


void php_print_credits(int flag)
{
	if (flag & PHP_CREDITS_FULLPAGE) {
		PUTS("<html><head><title>PHP Credits</title></head><body><center>\n");
	}

	PUTS("<h1>PHP 4.0 Credits</h1>\n");

	if (flag & PHP_CREDITS_GROUP) {
		/* Group */

		PUTS("<table border=5 width=\"600\">\n");
		php_info_print_table_header(1, "PHP Group");
		php_info_print_table_row(1, "Thies C. Arntzen, Stig Bakken, Andi Gutmans, Rasmus Lerdorf, \
					Sascha Schumann, Zeev Suraski, Jim Winstead, Andrei Zmievski");
		PUTS("</table>\n");
	}

	if (flag & PHP_CREDITS_GENERAL) {
		/* Language */

		PUTS("<table border=5 width=\"600\">\n");
		PUTS("<tr><th colspan=\"2\" bgcolor=\"" PHP_HEADER_COLOR "\">PHP 4.0 Authors</th></tr>\n");
		php_info_print_table_header(2, "Contribution", "Authors");
		CREDIT_LINE("Zend Scripting Language Engine", "Andi Gutmans, Zeev Suraski");
		CREDIT_LINE("Extension Module API", "Andi Gutmans, Zeev Suraski");
		CREDIT_LINE("UNIX Build and Modularization", "Stig Bakken, Sascha Schumann");
		CREDIT_LINE("Win32 Port", "Shane Caraveo, Zeev Suraski");
		CREDIT_LINE("Server API (SAPI) Abstraction Layer", "Andi Gutmans, Shane Caraveo, Zeev Suraski");
		PUTS("</table>\n");
	}

	if (flag & PHP_CREDITS_GENERAL) {
		/* SAPI Modules */

		PUTS("<table border=5 width=\"600\">\n");
		PUTS("<tr><th colspan=\"2\" bgcolor=\"" PHP_HEADER_COLOR "\">SAPI Modules</th></tr>\n");
		php_info_print_table_header(2, "Contribution", "Authors");
		CREDIT_LINE("Apache", "Rasmus Lerdorf, Zeev Suraski");
		CREDIT_LINE("ISAPI", "Andi Gutmans, Zeev Suraski");
		CREDIT_LINE("CGI", "Rasmus Lerdorf, Stig Bakken");
		CREDIT_LINE("AOLserver", "Sascha Schumann");
		CREDIT_LINE("Java Servlet", "Sam Ruby");
		CREDIT_LINE("Roxen", "David Hedbor");
		CREDIT_LINE("thttpd", "Sascha Schumann");
		PUTS("</table>\n");
	}

	if (flag & PHP_CREDITS_MODULES) {
		/* Modules */

		PUTS("<table border=5 width=\"600\">\n");
		PUTS("<tr><th colspan=\"2\" bgcolor=\"" PHP_HEADER_COLOR "\">Module Authors</th></tr>\n");
		php_info_print_table_header(2, "Module", "Authors");
		CREDIT_LINE("Apache", "Rasmus Lerdorf, Stig Bakken, David Sklar");
		CREDIT_LINE("BC Math", "Andi Gutmans");
		CREDIT_LINE("CyberCash", "Evan Klinger");
		CREDIT_LINE("Win32 COM", "Zeev Suraski");
		CREDIT_LINE("DAV", "Stig Bakken");
		CREDIT_LINE("DBA", "Sascha Schumann");
		CREDIT_LINE("DBM", "Rasmus Lerdorf, Jim Winstead");
		CREDIT_LINE("dBase", "Jim Winstead");
		CREDIT_LINE("FDF", "Uwe Steinmann");
		CREDIT_LINE("FilePro", "Chad Robinson");
		CREDIT_LINE("GD imaging", "Rasmus Lerdorf, Stig Bakken, Jim Winstead");
		CREDIT_LINE("GetText", "Alex Plotnick");
		CREDIT_LINE("HyperWave", "Uwe Steinmann");
		CREDIT_LINE("IMAP", "Rex Logan, Mark Musone, Brian Wang, Kaj-Michael Lang, Antoni Pamies Olive, Rasmus Lerdorf");
		CREDIT_LINE("Informix", "Danny Heijl, Christian Cartus, Jouni Ahto");
		CREDIT_LINE("Java", "Sam Ruby");
		CREDIT_LINE("Interbase", "Jouni Ahto");
		CREDIT_LINE("LDAP", "Amitay Isaacs, Eric Warnke, Rasmus Lerdorf, Gerrit Thomson");
		CREDIT_LINE("mcrypt", "Sascha Schumann");
		CREDIT_LINE("mhash", "Sascha Schumann");
		CREDIT_LINE("MS SQL", "Frank M. Kromann");
		CREDIT_LINE("mSQL", "Zeev Suraski");
		CREDIT_LINE("MySQL", "Zeev Suraski");
		CREDIT_LINE("OCI8", "Stig Bakken, Thies C. Arntzen");
		CREDIT_LINE("ODBC", "Stig Bakken, Andreas Karajannis, Frank M. Kromann");
		CREDIT_LINE("Oracle", "Stig Bakken, Mitch Golden, Rasmus Lerdorf, Andreas Karajannis, Thies C. Arntzen");
		CREDIT_LINE("Perl Compatible Regexps", "Andrei Zmievski");
		CREDIT_LINE("PDF", "Uwe Steinmann");
		CREDIT_LINE("PostgreSQL", "Jouni Ahto, Zeev Suraski");
		CREDIT_LINE("Sessions", "Sascha Schumann, Andrei Zmievski");
		CREDIT_LINE("SNMP", "Rasmus Lerdorf");
		CREDIT_LINE("Sybase", "Zeev Suraski");
		CREDIT_LINE("System V Shared Memory", "Christian Cartus");
		CREDIT_LINE("System V Semaphores", "Tom May");
		CREDIT_LINE("WDDX", "Andrei Zmievski");
		CREDIT_LINE("XML", "Stig Bakken, Thies C. Arntzen");
		CREDIT_LINE("Yellow Pages", "Stephanie Wehner");
		CREDIT_LINE("Zlib", "Rasmus Lerdorf, Stefan Roehrich");
		PUTS("</table>\n");
	}

	if (flag & PHP_CREDITS_DOCS) {
		/* Documentation */
	}

	if (flag & PHP_CREDITS_FULLPAGE) {
		PUTS("</center></body></html>\n");
	}
}


PHPAPI void php_info_print_table_header(int num_cols, ...)
{
	int i;
	va_list row_elements;
	char *row_element;

	va_start(row_elements, num_cols);

	php_printf("<tr>");
	for (i=0; i<num_cols; i++) {
		row_element = va_arg(row_elements, char *);
		if (!row_element || !*row_element) {
			row_element = "&nbsp;";
		}
		php_printf("<th bgcolor=\"" PHP_HEADER_COLOR "\" valign=\"top\">%s</th>", row_element);
	}
	php_printf("</tr>\n");

	va_end(row_elements);
}


PHPAPI void php_info_print_table_row(int num_cols, ...)
{
	int i;
	va_list row_elements;
	char *color = PHP_ENTRY_NAME_COLOR;
	char *row_element;

	va_start(row_elements, num_cols);

	php_printf("<tr>");
	for (i=0; i<num_cols; i++) {
		row_element = va_arg(row_elements, char *);
		if (!row_element || !*row_element) {
			row_element = "&nbsp;";
		}
		php_printf("<td bgcolor=\"%s\" valign=\"top\">%s%s%s</td>", 
			color, (i==0?"<b>":""), row_element, (i==0?"</b>":""));
		color = PHP_CONTENTS_COLOR;
	}
	php_printf("</tr>\n");

	va_end(row_elements);
}



void register_phpinfo_constants(INIT_FUNC_ARGS)
{
	REGISTER_LONG_CONSTANT("INFO_GENERAL", PHP_INFO_GENERAL, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_CREDITS", PHP_INFO_CREDITS, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_CONFIGURATION", PHP_INFO_CONFIGURATION, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_MODULES", PHP_INFO_MODULES, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_ENVIRONMENT", PHP_INFO_ENVIRONMENT, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_VARIABLES", PHP_INFO_VARIABLES, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_LICENSE", PHP_INFO_LICENSE, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_ALL", PHP_INFO_ALL, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_GROUP",	PHP_CREDITS_GROUP, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_GENERAL",	PHP_CREDITS_GENERAL, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_SAPI",	PHP_CREDITS_SAPI, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_MODULES",	PHP_CREDITS_MODULES, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_DOCS",	PHP_CREDITS_DOCS, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_FULLPAGE",	PHP_CREDITS_FULLPAGE, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_ALL",	PHP_CREDITS_ALL, CONST_PERSISTENT|CONST_CS);
}


/* {{{ proto void phpinfo(void)

   Output a page of useful information about PHP and the current request */
PHP_FUNCTION(phpinfo)
{
	int flag;
	zval **flag_arg;


	switch (ARG_COUNT(ht)) {
		case 0:
			flag = 0xFFFFFFFF;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &flag_arg)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(flag_arg);
			flag = (*flag_arg)->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	php_print_info(flag);
	RETURN_TRUE;
}

/* }}} */

/* {{{ proto string phpversion(void)
   Return the current PHP version */
PHP_FUNCTION(phpversion)
{
    RETURN_STRING(PHP_VERSION,1);
}
/* }}} */


/* {{{ proto void phpcredits(int)

   Prints the list of people who've contributed to the PHP project */
PHP_FUNCTION(phpcredits)
{
	int flag;
	zval **flag_arg;


	switch (ARG_COUNT(ht)) {
		case 0:
			flag = 0xFFFFFFFF;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &flag_arg)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(flag_arg);
			flag = (*flag_arg)->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	php_print_credits(flag);
	RETURN_TRUE;
}

/* }}} */


PHP_FUNCTION(php_logo_guid)
{
	RETURN_STRINGL(PHP_LOGO_GUID, sizeof(PHP_LOGO_GUID)-1, 1);
}


PHP_FUNCTION(zend_logo_guid)
{
	RETURN_STRINGL(ZEND_LOGO_GUID, sizeof(ZEND_LOGO_GUID)-1, 1);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
