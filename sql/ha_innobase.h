/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB
   && Innobase Oy
   
   -This file is modified from ha_berkeley.h of MySQL distribution-

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifdef __GNUC__
#pragma interface			/* gcc class implementation */
#endif

/* This file defines the Innobase handler: the interface between MySQL and
Innobase */

extern "C" {
#include <data0types.h>
#include <dict0types.h>
#include <row0types.h>
}

typedef struct st_innobase_share {
  THR_LOCK lock;
  pthread_mutex_t mutex;
  char *table_name;
  uint table_name_length,use_count;
} INNOBASE_SHARE;


/* The class defining a handle to an Innobase table */
class ha_innobase: public handler
{
  	void*	innobase_table_handle;	/* (dict_table_t*) pointer to a table
					in Innobase data dictionary cache */
	void*	innobase_prebuilt;	/* (row_prebuilt_t*) prebuilt
					structs in Innobase, used to save
					CPU */
	THD*		user_thd;	/* the thread handle of the user
					currently using the handle; this is
					set in external_lock function */
  	THR_LOCK_DATA 	lock;
	INNOBASE_SHARE  *share;

  	gptr 		alloc_ptr;
	byte*		rec_buff;	/* buffer used in converting
					rows from MySQL format
					to Innobase format */
  	byte*		upd_buff;	/* buffer used in updates */
  	byte*		key_val_buff;	/* buffer used in converting
  					search key values from MySQL format
  					to Innobase format */
	uint		ref_stored_len;	/* length of the key value stored to
					'ref' buffer of the handle, if any */
  	ulong 		int_option_flag;
  	uint 		primary_key;
	uint		last_dup_key;

	uint		last_match_mode;/* match mode of the latest search:
					ROW_SEL_EXACT or
					ROW_SEL_EXACT_PREFIX or undefined */
	
	ulong max_row_length(const byte *buf);

	uint store_key_val_for_row(uint keynr, char* buff, const byte* record);
  	void convert_row_to_innobase(dtuple_t* row, char* record);
	void convert_row_to_mysql(char*	record, dtuple_t* row);
	dtuple_t* convert_key_to_innobase(dtuple_t* tuple, byte* buf,
					dict_index_t* index,
					KEY* key, byte*	key_ptr, int key_len);
	int calc_row_difference(upd_t* uvect, byte* old_row, byte* new_row);
	int update_thd(THD* thd);
	int change_active_index(uint keynr);
	int general_fetch(byte* buf, uint direction, uint match_mode);

	/* Init values for the class: */
 public:
  	ha_innobase(TABLE *table): handler(table),
		rec_buff(0),
    		int_option_flag(HA_READ_NEXT | HA_READ_PREV | HA_READ_ORDER |
		    HA_REC_NOT_IN_SEQ |
		    HA_KEYPOS_TO_RNDPOS | HA_LASTKEY_ORDER |
		    HA_HAVE_KEY_READ_ONLY | HA_READ_NOT_EXACT_KEY |
		    HA_LONGLONG_KEYS | HA_NULL_KEY | HA_NO_BLOBS |
		    HA_NOT_EXACT_COUNT |
		    HA_NO_WRITE_DELAYED |
		    HA_PRIMARY_KEY_IN_READ_INDEX | HA_DROP_BEFORE_CREATE),
    		last_dup_key((uint) -1)
  	{
  	}
  	~ha_innobase() {}

  	const char* table_type() const { return("Innobase");}
  	const char** bas_ext() const;
 	ulong option_flag() const { return int_option_flag; }
  	uint max_record_length() const { return HA_MAX_REC_LENGTH; }
  	uint max_keys()          const { return MAX_KEY; }
  	uint max_key_parts()     const { return MAX_REF_PARTS; }
  	uint max_key_length()    const { return MAX_KEY_LENGTH; }
  	bool fast_key_read()	 { return 1;}
  	bool has_transactions()  { return 1;}

  	int open(const char *name, int mode, int test_if_locked);
  	void initialize(void);
  	int close(void);
  	double scan_time();

  	int write_row(byte * buf);
  	int update_row(const byte * old_data, byte * new_data);
  	int delete_row(const byte * buf);

  	int index_init(uint index);
  	int index_end();
  	int index_read(byte * buf, const byte * key,
		 	uint key_len, enum ha_rkey_function find_flag);
  	int index_read_idx(byte * buf, uint index, const byte * key,
		     	uint key_len, enum ha_rkey_function find_flag);
  	int index_next(byte * buf);
  	int index_next_same(byte * buf, const byte *key, uint keylen);
  	int index_prev(byte * buf);
  	int index_first(byte * buf);
  	int index_last(byte * buf);

  	int rnd_init(bool scan=1);
  	int rnd_end();
  	int rnd_next(byte *buf);
  	int rnd_pos(byte * buf, byte *pos);

  	void position(const byte *record);
  	void info(uint);
  	int extra(enum ha_extra_function operation);
  	int reset(void);
  	int external_lock(THD *thd, int lock_type);
  	void position(byte *record);
  	ha_rows records_in_range(int inx,
			   const byte *start_key,uint start_key_len,
			   enum ha_rkey_function start_search_flag,
			   const byte *end_key,uint end_key_len,
			   enum ha_rkey_function end_search_flag);

  	int create(const char *name, register TABLE *form,
					HA_CREATE_INFO *create_info);
  	int delete_table(const char *name);
	int rename_table(const char* from, const char* to);

  	THR_LOCK_DATA **store_lock(THD *thd, THR_LOCK_DATA **to,
			     		enum thr_lock_type lock_type);
};

extern bool innobase_skip;
extern uint innobase_init_flags, innobase_lock_type;
extern ulong innobase_cache_size;
extern char *innobase_home, *innobase_tmpdir, *innobase_logdir;
extern long innobase_lock_scan_time;
extern long innobase_mirrored_log_groups, innobase_mirrored_log_groups;
extern long innobase_log_file_size, innobase_log_buffer_size;
extern long innobase_buffer_pool_size, innobase_additional_mem_pool_size;
extern long innobase_file_io_threads;
extern char *innobase_data_home_dir, *innobase_data_file_path;
extern char *innobase_log_group_home_dir, *innobase_log_arch_dir;
extern bool innobase_flush_log_at_trx_commit,innobase_log_archive;

extern TYPELIB innobase_lock_typelib;

bool innobase_init(void);
bool innobase_end(void);
bool innobase_flush_logs(void);

int innobase_commit(THD *thd);
int innobase_rollback(THD *thd);
int innobase_close_connection(THD *thd);

