#include "vnstat_tests.h"
#include "config_tests.h"
#include "common.h"
#include "cfg.h"
#include "ibw.h"

START_TEST(validatecfg_default)
{
	defaultcfg();
	validatecfg();
}
END_TEST

START_TEST(printcfgfile_default)
{
	defaultcfg();
	ck_assert_int_eq(ibwadd("name1", 1), 1);
	ck_assert_int_eq(ibwadd("name2", 2), 1);
	suppress_output();
	printcfgfile();
}
END_TEST

START_TEST(loadcfg_included_default)
{
	ck_assert_int_eq(loadcfg(CFGFILE), 1);
}
END_TEST

START_TEST(loadcfg_no_file)
{
	ck_assert_int_eq(loadcfg(""), 1);
}
END_TEST

START_TEST(loadcfg_nonexistent_file)
{
	suppress_output();
	ck_assert_int_eq(loadcfg("_nosuchfile_"), 0);
}
END_TEST

START_TEST(loadcfg_not_a_cfgfile)
{
	ck_assert_int_eq(loadcfg("Makefile"), 1);
}
END_TEST

START_TEST(ibwloadcfg_included_default)
{
	ck_assert_int_eq(ibwloadcfg(CFGFILE), 1);
}
END_TEST

START_TEST(ibwloadcfg_no_file)
{
	ck_assert_int_eq(ibwloadcfg(""), 1);
}
END_TEST

START_TEST(ibwloadcfg_nonexistent_file)
{
	suppress_output();
	ck_assert_int_eq(ibwloadcfg("_nosuchfile_"), 0);
}
END_TEST

START_TEST(ibwloadcfg_not_a_cfgfile)
{
	ck_assert_int_eq(ibwloadcfg("Makefile"), 1);
}
END_TEST

START_TEST(ibwget_with_empty_list_and_no_maxbw)
{
	uint32_t limit;
	cfg.maxbw = 0;
	ck_assert_int_eq(ibwget("does_not_exist", &limit), 0);
}
END_TEST

START_TEST(ibwget_with_empty_list_and_maxbw)
{
	int ret;
	uint32_t limit;
	cfg.maxbw = 10;
	ret  = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 10);
}
END_TEST

START_TEST(ibwget_from_config)
{
	int ret;
	uint32_t limit;
	ck_assert_int_eq(loadcfg(CFGFILE), 1);
	ck_assert_int_eq(ibwloadcfg(CFGFILE), 1);
	cfg.maxbw = 10;
	ret  = ibwget("ethnone", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 8);
}
END_TEST

START_TEST(ibwadd_single_success)
{
	int ret;
	uint32_t limit;
	cfg.maxbw = 0;
	ck_assert_int_eq(ibwadd("newinterface", 1), 1);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
	ret = ibwget("newinterface", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
}
END_TEST

START_TEST(ibwadd_multi_success)
{
	int ret;
	uint32_t limit;
	cfg.maxbw = 0;
	ck_assert_int_eq(ibwadd("name1", 1), 1);
	ck_assert_int_eq(ibwadd("name2", 2), 1);
	ck_assert_int_eq(ibwadd("name3", 3), 1);
	ck_assert_int_eq(ibwadd("name4", 2), 1);
	ck_assert_int_eq(ibwadd("name5", 1), 1);
	ck_assert_int_eq(ibwadd("name6", 10), 1);

	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
	ret = ibwget("name3", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 3);
	ret = ibwget("name4", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 2);
	ret = ibwget("name6", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 10);
	ret = ibwget("name2", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 2);
	ret = ibwget("name5", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
}
END_TEST

START_TEST(ibwadd_update_success)
{
	int ret;
	uint32_t limit;
	cfg.maxbw = 0;
	ck_assert_int_eq(ibwadd("name1", 1), 1);
	ck_assert_int_eq(ibwadd("name2", 2), 1);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);

	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
	ret = ibwget("name2", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 2);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);

	ck_assert_int_eq(ibwadd("name2", 5), 1);
	ck_assert_int_eq(ibwadd("name1", 4), 1);

	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 4);
	ret = ibwget("name2", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 5);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
}
END_TEST

START_TEST(ibwflush_success)
{
	int ret;
	uint32_t limit;
	cfg.maxbw = 0;
	ck_assert_int_eq(ibwadd("name1", 1), 1);
	ck_assert_int_eq(ibwadd("name2", 2), 1);

	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
	ret = ibwget("name2", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 2);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);

	ibwflush();

	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
	ret = ibwget("name2", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);

	ck_assert_int_eq(ibwadd("name1", 1), 1);
	ck_assert_int_eq(ibwadd("name2", 2), 1);

	ret = ibwget("name1", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 1);
	ret = ibwget("name2", &limit);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(limit, 2);
	ret = ibwget("does_not_exist", &limit);
	ck_assert_int_eq(ret, 0);
	ck_assert_int_eq(limit, 0);
}
END_TEST

START_TEST(ibwlist_empty)
{
	suppress_output();
	ibwlist();
}
END_TEST

START_TEST(ibwlist_filled)
{
	cfg.maxbw = 0;
	ck_assert_int_eq(ibwadd("name1", 1), 1);
	ck_assert_int_eq(ibwadd("name2", 2), 1);
	suppress_output();
	ibwlist();
}
END_TEST

void add_config_tests(Suite *s)
{
	TCase *tc_config = tcase_create("Config");
	tcase_add_test(tc_config, validatecfg_default);
	tcase_add_test(tc_config, printcfgfile_default);
	tcase_add_test(tc_config, loadcfg_included_default);
	tcase_add_test(tc_config, loadcfg_no_file);
	tcase_add_test(tc_config, loadcfg_nonexistent_file);
	tcase_add_test(tc_config, loadcfg_not_a_cfgfile);
	tcase_add_test(tc_config, ibwloadcfg_included_default);
	tcase_add_test(tc_config, ibwloadcfg_no_file);
	tcase_add_test(tc_config, ibwloadcfg_nonexistent_file);
	tcase_add_test(tc_config, ibwloadcfg_not_a_cfgfile);
	tcase_add_test(tc_config, ibwget_with_empty_list_and_no_maxbw);
	tcase_add_test(tc_config, ibwget_with_empty_list_and_maxbw);
	tcase_add_test(tc_config, ibwget_from_config);
	tcase_add_test(tc_config, ibwadd_single_success);
	tcase_add_test(tc_config, ibwadd_multi_success);
	tcase_add_test(tc_config, ibwadd_update_success);
	tcase_add_test(tc_config, ibwflush_success);
	tcase_add_test(tc_config, ibwlist_empty);
	tcase_add_test(tc_config, ibwlist_filled);
	suite_add_tcase(s, tc_config);
}
