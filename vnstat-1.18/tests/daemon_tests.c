#include "vnstat_tests.h"
#include "daemon_tests.h"
#include "common.h"
#include "dbaccess.h"
#include "dbcache.h"
#include "cfg.h"
#include "fs.h"
#include "daemon.h"

START_TEST(debugtimestamp_does_not_exit)
{
	suppress_output();
	debugtimestamp();
}
END_TEST

START_TEST(addinterfaces_does_nothing_with_no_files)
{
	linuxonly;

	defaultcfg();
	suppress_output();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);

	ck_assert_int_eq(addinterfaces(TESTDBDIR, 0), 0);
}
END_TEST

START_TEST(addinterfaces_adds_interfaces)
{
	int ret;
	linuxonly;

	defaultcfg();
	suppress_output();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	fake_proc_net_dev("w", "ethone", 1, 2, 3, 4);
	fake_proc_net_dev("a", "lo0", 0, 0, 0, 0);
	fake_proc_net_dev("a", "ethtwo", 5, 6, 7, 8);
	fake_proc_net_dev("a", "sit0", 0, 0, 0, 0);

	ret = addinterfaces(TESTDBDIR, 0);
	ck_assert_int_eq(ret, 2);

	ck_assert_int_eq(check_dbfile_exists("ethone", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethone", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("ethtwo", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethtwo", sizeof(DATA)), 0);
}
END_TEST

START_TEST(addinterfaces_adds_only_new_interfaces)
{
	int ret;
	linuxonly;

	defaultcfg();
	suppress_output();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	fake_proc_net_dev("w", "ethone", 1, 2, 3, 4);
	fake_proc_net_dev("a", "lo0", 0, 0, 0, 0);
	fake_proc_net_dev("a", "ethtwo", 5, 6, 7, 8);
	fake_proc_net_dev("a", "sit0", 0, 0, 0, 0);

	ret = addinterfaces(TESTDBDIR, 0);
	ck_assert_int_eq(ret, 2);

	ck_assert_int_eq(check_dbfile_exists("ethone", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethone", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("ethtwo", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethtwo", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("eththree", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists(".eththree", sizeof(DATA)), 0);

	fake_proc_net_dev("a", "eththree", 9, 10, 11, 12);

	ret = addinterfaces(TESTDBDIR, 0);
	ck_assert_int_eq(ret, 1);

	ck_assert_int_eq(check_dbfile_exists("ethone", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethone", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("ethtwo", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethtwo", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("eththree", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".eththree", sizeof(DATA)), 0);
	ck_assert_int_eq(cachecount(), 0);
}
END_TEST

START_TEST(addinterfaces_adds_to_cache_when_running)
{
	int ret;
	linuxonly;

	defaultcfg();
	suppress_output();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	fake_proc_net_dev("w", "ethone", 1, 2, 3, 4);
	fake_proc_net_dev("a", "ethtwo", 5, 6, 7, 8);

	ck_assert_int_eq(cachecount(), 0);

	ret = addinterfaces(TESTDBDIR, 1);
	ck_assert_int_eq(ret, 2);
	ck_assert_int_eq(cachecount(), 2);

	ck_assert_int_eq(check_dbfile_exists("ethone", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethone", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("ethtwo", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethtwo", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("eththree", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists(".eththree", sizeof(DATA)), 0);

	fake_proc_net_dev("a", "eththree", 9, 10, 11, 12);

	ret = addinterfaces(TESTDBDIR, 1);
	ck_assert_int_eq(ret, 1);
	ck_assert_int_eq(cachecount(), 3);

	ck_assert_int_eq(check_dbfile_exists("ethone", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethone", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("ethtwo", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethtwo", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("eththree", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".eththree", sizeof(DATA)), 0);
}
END_TEST

START_TEST(initdstate_does_not_crash)
{
	DSTATE s;
	defaultcfg();
	initdstate(&s);
}
END_TEST

START_TEST(preparedatabases_exits_with_no_database_dir)
{
	DSTATE s;

	linuxonly_exit;

	defaultcfg();
	initdstate(&s);
	suppress_output();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);

	preparedatabases(&s);
}
END_TEST

START_TEST(preparedatabases_exits_with_no_databases)
{
	DSTATE s;

	linuxonly_exit;

	defaultcfg();
	initdstate(&s);
	suppress_output();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);

	preparedatabases(&s);
}
END_TEST

START_TEST(preparedatabases_exits_with_no_databases_and_noadd)
{
	DSTATE s;

	linuxonly_exit;

	defaultcfg();
	initdstate(&s);
	s.noadd = 1;
	suppress_output();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);

	preparedatabases(&s);
}
END_TEST

START_TEST(preparedatabases_with_no_databases_creates_databases)
{
	DSTATE s;

	linuxonly;

	defaultcfg();
	initdstate(&s);
	suppress_output();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	fake_proc_net_dev("w", "ethone", 1, 2, 3, 4);
	fake_proc_net_dev("a", "lo0", 0, 0, 0, 0);
	fake_proc_net_dev("a", "ethtwo", 5, 6, 7, 8);
	fake_proc_net_dev("a", "sit0", 0, 0, 0, 0);

	preparedatabases(&s);

	ck_assert_int_eq(check_dbfile_exists("ethone", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethone", sizeof(DATA)), 0);
	ck_assert_int_eq(check_dbfile_exists("ethtwo", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".ethtwo", sizeof(DATA)), 0);
}
END_TEST

START_TEST(setsignaltraps_does_not_exit)
{
	intsignal = 1;
	setsignaltraps();
	ck_assert_int_eq(intsignal, 0);
}
END_TEST

START_TEST(filldatabaselist_exits_with_no_database_dir)
{
	DSTATE s;
	defaultcfg();
	initdstate(&s);
	disable_logprints();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);

	filldatabaselist(&s);
}
END_TEST

START_TEST(filldatabaselist_does_not_exit_with_empty_database_dir)
{
	DSTATE s;
	defaultcfg();
	initdstate(&s);
	disable_logprints();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	s.sync = 1;
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);

	filldatabaselist(&s);

	ck_assert_int_eq(s.dbcount, 0);
	ck_assert_int_eq(s.sync, 0);
	ck_assert_int_eq(s.updateinterval, 120);
}
END_TEST

START_TEST(filldatabaselist_adds_databases)
{
	DSTATE s;
	defaultcfg();
	initdstate(&s);
	disable_logprints();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	s.sync = 1;
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	ck_assert_int_eq(create_zerosize_dbfile("name1"), 1);
	ck_assert_int_eq(create_zerosize_dbfile("name2"), 1);
	ck_assert_int_eq(check_dbfile_exists("name1", 0), 1);
	ck_assert_int_eq(check_dbfile_exists(".name1", 0), 0);
	ck_assert_int_eq(check_dbfile_exists("name2", 0), 1);
	ck_assert_int_eq(check_dbfile_exists(".name2", 0), 0);

	filldatabaselist(&s);

	ck_assert_int_eq(cachecount(), 2);
	ck_assert_int_eq(cacheactivecount(), 2);
	ck_assert_int_eq(check_dbfile_exists("name1", 0), 1);
	ck_assert_int_eq(check_dbfile_exists(".name1", 0), 0);
	ck_assert_int_eq(check_dbfile_exists("name2", 0), 1);
	ck_assert_int_eq(check_dbfile_exists(".name2", 0), 0);
	ck_assert_int_eq(s.dbcount, 2);
	ck_assert_int_eq(s.sync, 0);
	ck_assert_int_eq(s.updateinterval, 0);
	ck_assert_int_eq(intsignal, 42);
}
END_TEST

START_TEST(adjustsaveinterval_with_empty_cache)
{
	DSTATE s;
	defaultcfg();
	initdstate(&s);
	s.saveinterval = 0;
	ck_assert_int_eq(cacheactivecount(), 0);

	adjustsaveinterval(&s);

	ck_assert_int_eq(s.saveinterval, cfg.offsaveinterval * 60);
}
END_TEST

START_TEST(adjustsaveinterval_with_filled_cache)
{
	DSTATE s;
	defaultcfg();
	initdb();
	initdstate(&s);
	s.saveinterval = 0;
	strcpy(data.interface, "name1");
	ck_assert_int_eq(cacheupdate(), 1);
	ck_assert_int_eq(cacheactivecount(), 1);

	adjustsaveinterval(&s);

	ck_assert_int_eq(s.saveinterval, cfg.saveinterval * 60);
}
END_TEST

START_TEST(checkdbsaveneed_has_no_need)
{
	DSTATE s;
	initdstate(&s);
	s.dodbsave = 2;
	s.current = 10;
	s.prevdbsave = 0;
	s.saveinterval = 30;
	s.forcesave = 0;

	checkdbsaveneed(&s);

	ck_assert_int_eq(s.dodbsave, 0);
	ck_assert_int_ne(s.prevdbsave, s.current);
}
END_TEST

START_TEST(checkdbsaveneed_is_forced)
{
	DSTATE s;
	initdstate(&s);
	s.dodbsave = 2;
	s.current = 10;
	s.prevdbsave = 0;
	s.saveinterval = 30;
	s.forcesave = 1;

	checkdbsaveneed(&s);

	ck_assert_int_eq(s.dodbsave, 1);
	ck_assert_int_eq(s.prevdbsave, s.current);
	ck_assert_int_eq(s.forcesave, 0);
}
END_TEST

START_TEST(checkdbsaveneed_needs)
{
	DSTATE s;
	initdstate(&s);
	s.dodbsave = 2;
	s.current = 60;
	s.prevdbsave = 5;
	s.saveinterval = 30;
	s.forcesave = 0;

	checkdbsaveneed(&s);

	ck_assert_int_eq(s.dodbsave, 1);
	ck_assert_int_eq(s.prevdbsave, s.current);
	ck_assert_int_eq(s.forcesave, 0);
}
END_TEST

START_TEST(datalist_cacheget_with_no_database)
{
	DSTATE s;
	defaultcfg();
	initdb();
	initdstate(&s);
	disable_logprints();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	ck_assert_int_eq(cacheadd("name1", 0), 1);
	s.dbcount = 1;
	s.dbhash = 123;
	s.datalist = dataptr;

	ck_assert_int_eq(datalist_cacheget(&s), 0);
	ck_assert_int_eq(s.dbhash, 123);
	ck_assert_int_eq(s.datalist->filled, 0);
}
END_TEST

START_TEST(datalist_cacheget_with_database)
{
	DSTATE s;
	defaultcfg();
	initdb();
	initdstate(&s);
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	ck_assert_int_eq(cacheadd("name1", 0), 1);
	strncpy_nt(data.interface, "name1", 32);
	ck_assert_int_eq(writedb("name1", TESTDBDIR, 1), 1);
	ck_assert_int_eq(check_dbfile_exists("name1", sizeof(DATA)), 1);
	s.dbcount = 1;
	s.dbhash = 123;
	s.datalist = dataptr;

	ck_assert_int_eq(datalist_cacheget(&s), 1);
	ck_assert_int_eq(s.dbhash, 0);
	ck_assert_int_eq(s.datalist->filled, 1);
}
END_TEST

START_TEST(datalist_getifinfo_with_disabled_interface)
{
	DSTATE s;

	linuxonly;

	initdb();
	initdstate(&s);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	data.active = 0;

	datalist_getifinfo(&s);

	ck_assert_int_eq(data.active, 0);
}
END_TEST

START_TEST(datalist_getifinfo_with_enabled_unavailable_interface)
{
	DSTATE s;

	linuxonly;

	initdb();
	initdstate(&s);
	disable_logprints();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	strncpy_nt(data.interface, "name1", 32);
	data.active = 1;

	datalist_getifinfo(&s);

	ck_assert_int_eq(data.active, 0);
}
END_TEST

START_TEST(datalist_getifinfo_with_interface_sync)
{
	DSTATE s;

	linuxonly;

	initdb();
	initdstate(&s);
	disable_logprints();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	fake_proc_net_dev("w", "name1", 1025, 2050, 30, 40);
	strncpy_nt(data.interface, "name1", 32);
	ck_assert_int_eq(cacheadd("name1", 1), 1);
	data.active = 1;
	data.currx = 1;
	data.curtx = 2;
	s.dbcount = 1;
	s.datalist = dataptr;
	ck_assert_int_eq(s.datalist->sync, 1);
	ck_assert_int_eq(data.currx, 1);
	ck_assert_int_eq(data.curtx, 2);
	ck_assert_int_eq(data.totalrxk, 0);
	ck_assert_int_eq(data.totaltxk, 0);

	datalist_getifinfo(&s);

	ck_assert_int_eq(data.active, 1);
	ck_assert_int_eq(data.totalrxk, 0);
	ck_assert_int_eq(data.totaltxk, 0);
	ck_assert_int_eq(data.currx, 1025);
	ck_assert_int_eq(data.curtx, 2050);
	ck_assert_int_eq(s.datalist->sync, 0);
}
END_TEST

START_TEST(datalist_getifinfo_with_interface_and_no_sync)
{
	DSTATE s;

	linuxonly;

	initdb();
	initdstate(&s);
	disable_logprints();
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	fake_proc_net_dev("w", "name1", 1025, 2050, 30, 40);
	strncpy_nt(data.interface, "name1", 32);
	ck_assert_int_eq(cacheadd("name1", 0), 1);
	data.active = 1;
	data.currx = 1;
	data.curtx = 2;
	s.dbcount = 1;
	s.datalist = dataptr;
	ck_assert_int_eq(s.datalist->sync, 0);
	ck_assert_int_eq(data.currx, 1);
	ck_assert_int_eq(data.curtx, 2);
	ck_assert_int_eq(data.totalrxk, 0);
	ck_assert_int_eq(data.totaltxk, 0);

	datalist_getifinfo(&s);

	ck_assert_int_eq(data.active, 1);
	ck_assert_int_eq(data.totalrxk, 1);
	ck_assert_int_eq(data.totaltxk, 2);
	ck_assert_int_eq(data.currx, 1025);
	ck_assert_int_eq(data.curtx, 2050);
	ck_assert_int_eq(s.datalist->sync, 0);
}
END_TEST

START_TEST(datalist_timevalidation_in_normal_time)
{
	DSTATE s;
	initdb();
	initdstate(&s);
	data.lastupdated = time(NULL);
	s.current = time(NULL);

	ck_assert_int_eq(datalist_timevalidation(&s), 1);
	ck_assert_int_eq(data.lastupdated, s.current);
	s.current++;
	ck_assert_int_eq(datalist_timevalidation(&s), 1);
	ck_assert_int_eq(data.lastupdated, s.current);
}
END_TEST

START_TEST(datalist_timevalidation_in_future_time)
{
	DSTATE s;
	initdb();
	initdstate(&s);
	data.lastupdated = time(NULL)+10;
	s.current = time(NULL);

	ck_assert_int_eq(datalist_timevalidation(&s), 0);
	ck_assert_int_ne(data.lastupdated, s.current);
}
END_TEST

START_TEST(datalist_timevalidation_in_too_future_time)
{
	DSTATE s;
	initdb();
	initdstate(&s);
	disable_logprints();
	data.lastupdated = time(NULL)+90000;
	s.current = time(NULL);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);

	ck_assert_int_eq(datalist_timevalidation(&s), 0);
	ck_assert_int_ne(data.lastupdated, s.current);
}
END_TEST

START_TEST(datalist_writedb_does_not_save_unless_requested)
{
	DSTATE s;
	initdstate(&s);
	s.dodbsave = 0;
	s.dbsaved = 0;

	ck_assert_int_eq(datalist_writedb(&s), 1);
	ck_assert_int_eq(s.dbsaved, 0);
}
END_TEST

START_TEST(datalist_writedb_detects_missing_database_file)
{
	DSTATE s;
	initdstate(&s);
	s.dodbsave = 1;
	s.dbsaved = 0;
	disable_logprints();
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(cacheadd("name1", 0), 1);
	s.datalist = dataptr;

	ck_assert_int_eq(datalist_writedb(&s), 0);
	ck_assert_int_eq(s.dbsaved, 0);
}
END_TEST

START_TEST(datalist_writedb_writes_database_file)
{
	DSTATE s;
	initdb();
	initdstate(&s);
	s.dodbsave = 1;
	s.dbsaved = 0;
	disable_logprints();
	strncpy_nt(data.interface, "name1", 32);
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	ck_assert_int_eq(create_zerosize_dbfile("name1"), 1);
	ck_assert_int_eq(cacheadd("name1", 0), 1);
	s.datalist = dataptr;

	ck_assert_int_eq(datalist_writedb(&s), 1);
	ck_assert_int_eq(s.dbsaved, 1);
	ck_assert_int_eq(check_dbfile_exists("name1", sizeof(DATA)), 1);
	ck_assert_int_eq(check_dbfile_exists(".name1", 0), 1);
}
END_TEST

START_TEST(processdatalist_empty_does_nothing)
{
	DSTATE s;
	initdstate(&s);

	processdatalist(&s);
}
END_TEST

START_TEST(processdatalist_filled_does_things)
{
	DSTATE s;

	linuxonly;

	initdb();
	initdstate(&s);
	disable_logprints();
	s.current = time(NULL);
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(clean_testdbdir(), 1);
	ck_assert_int_eq(create_zerosize_dbfile("ethnormal"), 1);
	ck_assert_int_eq(create_zerosize_dbfile("ethunavailable"), 1);
	ck_assert_int_eq(create_zerosize_dbfile("ethbogus"), 1);
	ck_assert_int_eq(create_zerosize_dbfile("ethfuture"), 1);
	fake_proc_net_dev("w", "ethbogus", 1, 2, 3, 4);
	fake_proc_net_dev("a", "ethnormal", 1024, 2048, 30, 40);
	fake_proc_net_dev("a", "ethnodb", 2048, 3072, 40, 50);
	fake_proc_net_dev("a", "ethfuture", 3072, 4096, 50, 60);

	strcpy(data.interface, "ethnormal");
	ck_assert_int_eq(cacheupdate(), 1);
	strcpy(data.interface, "ethunavailable");
	ck_assert_int_eq(cacheupdate(), 1);
	strcpy(data.interface, "ethnodb");
	ck_assert_int_eq(cacheupdate(), 1);
	strcpy(data.interface, "ethfuture");
	data.lastupdated = time(NULL)+10;
	ck_assert_int_eq(cacheupdate(), 1);
	strcpy(data.interface, "ethbogus");
	data.lastupdated = time(NULL);
	data.version = 0;
	ck_assert_int_eq(cacheupdate(), 1);
	strcpy(data.interface, "foo");
	s.dbcount = 5;
	s.dodbsave = 1;
	s.datalist = dataptr;

	processdatalist(&s);
	ck_assert_int_eq(s.dbcount, 4);
	ck_assert_int_eq(cachegetname("ethnodb"), 0);
	ck_assert_int_eq(cachegetname("ethunavailable"), 1);
	ck_assert_int_eq(data.active, 0);
	ck_assert_int_eq(cachegetname("ethnormal"), 1);
	ck_assert_int_eq(data.active, 1);
	ck_assert_int_eq(data.currx, 1024);
	ck_assert_int_eq(data.curtx, 2048);
	ck_assert_int_eq(cachegetname("ethfuture"), 1);
	ck_assert_int_eq(data.active, 1);
	ck_assert_int_eq(data.currx, 0);
	ck_assert_int_eq(data.curtx, 0);
	ck_assert_int_eq(cachegetname("ethbogus"), 1);
	ck_assert_int_eq(data.active, 1);
	ck_assert_int_eq(data.currx, 0);
	ck_assert_int_eq(data.curtx, 0);
}
END_TEST

START_TEST(handleintsignals_handles_signals)
{
	DSTATE s;
	defaultcfg();
	initdstate(&s);
	s.running = 1;
	s.dbcount = 1;

	intsignal = 0;
	handleintsignals(&s);
	ck_assert_int_eq(intsignal, 0);
	ck_assert_int_eq(s.running, 1);
	ck_assert_int_eq(s.dbcount, 1);

	intsignal = 42;
	handleintsignals(&s);
	ck_assert_int_eq(intsignal, 0);
	ck_assert_int_eq(s.running, 1);
	ck_assert_int_eq(s.dbcount, 1);

	disable_logprints();

	intsignal = 43;
	handleintsignals(&s);
	ck_assert_int_eq(intsignal, 0);
	ck_assert_int_eq(s.running, 1);
	ck_assert_int_eq(s.dbcount, 1);

	intsignal = SIGTERM;
	handleintsignals(&s);
	ck_assert_int_eq(intsignal, 0);
	ck_assert_int_eq(s.running, 0);
	ck_assert_int_eq(s.dbcount, 1);

	s.running = 1;
	intsignal = SIGINT;
	handleintsignals(&s);
	ck_assert_int_eq(intsignal, 0);
	ck_assert_int_eq(s.running, 0);
	ck_assert_int_eq(s.dbcount, 1);

	s.running = 1;
	intsignal = SIGHUP;
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	handleintsignals(&s);
	ck_assert_int_eq(intsignal, 0);
	ck_assert_int_eq(s.running, 1);
	ck_assert_int_eq(s.dbcount, 0);
}
END_TEST

START_TEST(preparedirs_with_no_dir)
{
	char logdir[512], piddir[512];

	DSTATE s;
	initdstate(&s);
	defaultcfg();
	cfg.uselogging = 1;
	s.rundaemon = 1;
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	snprintf(logdir, 512, "%s/log/vnstat", TESTDIR);
	snprintf(piddir, 512, "%s/pid/vnstat", TESTDIR);
	snprintf(cfg.logfile, 512, "%s/vnstat.log", logdir);
	snprintf(cfg.pidfile, 512, "%s/vnstat.pid", piddir);

	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(direxists(TESTDBDIR), 0);
	ck_assert_int_eq(direxists(logdir), 0);
	ck_assert_int_eq(direxists(piddir), 0);
	preparedirs(&s);
	ck_assert_int_eq(direxists(TESTDBDIR), 1);
	ck_assert_int_eq(direxists(logdir), 1);
	ck_assert_int_eq(direxists(piddir), 1);
}
END_TEST

START_TEST(preparedirs_with_dir)
{
	char logdir[512], piddir[512];

	DSTATE s;
	initdstate(&s);
	defaultcfg();
	cfg.uselogging = 1;
	s.rundaemon = 1;
	strncpy_nt(s.dirname, TESTDBDIR, 512);
	snprintf(logdir, 512, "%s/log/vnstat", TESTDIR);
	snprintf(piddir, 512, "%s/pid/vnstat", TESTDIR);
	snprintf(cfg.logfile, 512, "%s/vnstat.log", logdir);
	snprintf(cfg.pidfile, 512, "%s/vnstat.pid", piddir);

	ck_assert_int_eq(remove_directory(TESTDIR), 1);
	ck_assert_int_eq(direxists(TESTDBDIR), 0);
	ck_assert_int_eq(mkpath(TESTDBDIR, 0775), 1);
	ck_assert_int_eq(direxists(TESTDBDIR), 1);
	ck_assert_int_eq(direxists(logdir), 0);
	ck_assert_int_eq(direxists(piddir), 0);
	preparedirs(&s);
	ck_assert_int_eq(direxists(TESTDBDIR), 1);
	ck_assert_int_eq(direxists(logdir), 1);
	ck_assert_int_eq(direxists(piddir), 1);
}
END_TEST

void add_daemon_tests(Suite *s)
{
	TCase *tc_daemon = tcase_create("Daemon");
	tcase_add_test(tc_daemon, debugtimestamp_does_not_exit);
	tcase_add_test(tc_daemon, initdstate_does_not_crash);
	tcase_add_test(tc_daemon, addinterfaces_does_nothing_with_no_files);
	tcase_add_test(tc_daemon, addinterfaces_adds_interfaces);
	tcase_add_test(tc_daemon, addinterfaces_adds_only_new_interfaces);
	tcase_add_test(tc_daemon, addinterfaces_adds_to_cache_when_running);
	tcase_add_exit_test(tc_daemon, preparedatabases_exits_with_no_database_dir, 1);
	tcase_add_exit_test(tc_daemon, preparedatabases_exits_with_no_databases, 1);
	tcase_add_exit_test(tc_daemon, preparedatabases_exits_with_no_databases_and_noadd, 1);
	tcase_add_test(tc_daemon, preparedatabases_with_no_databases_creates_databases);
	tcase_add_test(tc_daemon, setsignaltraps_does_not_exit);
	tcase_add_exit_test(tc_daemon, filldatabaselist_exits_with_no_database_dir, 1);
	tcase_add_test(tc_daemon, filldatabaselist_does_not_exit_with_empty_database_dir);
	tcase_add_test(tc_daemon, filldatabaselist_adds_databases);
	tcase_add_test(tc_daemon, adjustsaveinterval_with_empty_cache);
	tcase_add_test(tc_daemon, adjustsaveinterval_with_filled_cache);
	tcase_add_test(tc_daemon, checkdbsaveneed_has_no_need);
	tcase_add_test(tc_daemon, checkdbsaveneed_is_forced);
	tcase_add_test(tc_daemon, checkdbsaveneed_needs);
	tcase_add_test(tc_daemon, datalist_cacheget_with_no_database);
	tcase_add_test(tc_daemon, datalist_cacheget_with_database);
	tcase_add_test(tc_daemon, datalist_getifinfo_with_disabled_interface);
	tcase_add_test(tc_daemon, datalist_getifinfo_with_enabled_unavailable_interface);
	tcase_add_test(tc_daemon, datalist_getifinfo_with_interface_sync);
	tcase_add_test(tc_daemon, datalist_getifinfo_with_interface_and_no_sync);
	tcase_add_test(tc_daemon, datalist_timevalidation_in_normal_time);
	tcase_add_test(tc_daemon, datalist_timevalidation_in_future_time);
	tcase_add_exit_test(tc_daemon, datalist_timevalidation_in_too_future_time, 1);
	tcase_add_test(tc_daemon, datalist_writedb_does_not_save_unless_requested);
	tcase_add_test(tc_daemon, datalist_writedb_detects_missing_database_file);
	tcase_add_test(tc_daemon, datalist_writedb_writes_database_file);
	tcase_add_test(tc_daemon, processdatalist_empty_does_nothing);
	tcase_add_test(tc_daemon, processdatalist_filled_does_things);
	tcase_add_test(tc_daemon, handleintsignals_handles_signals);
	tcase_add_test(tc_daemon, preparedirs_with_no_dir);
	tcase_add_test(tc_daemon, preparedirs_with_dir);
	suite_add_tcase(s, tc_daemon);
}

int cachegetname(const char *iface)
{
	datanode *dn;
	dn = dataptr;

	while (dn != NULL) {
		if (strcmp(dn->data.interface, iface) == 0) {
			memcpy(&data, &dn->data, sizeof(data));
			return 1;
		}
		dn = dn->next;
	}
	return 0;
}
