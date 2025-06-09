// See LICENSE for license details.

#include "cachesim.h"
#include "common.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <queue>

cache_sim_t::cache_sim_t(size_t _sets, size_t _ways, size_t _linesz, const char* _name)
: sets(_sets), ways(_ways), linesz(_linesz), name(_name), log(false)
{
  init();
}

static void help()
{
  std::cerr << "Cache configurations must be of the form" << std::endl;
  std::cerr << "  sets:ways:blocksize" << std::endl;
  std::cerr << "where sets, ways, and blocksize are positive integers, with" << std::endl;
  std::cerr << "sets and blocksize both powers of two and blocksize at least 8." << std::endl;
  exit(1);
}

cache_sim_t* cache_sim_t::construct(const char* config, const char* name)
{
  const char* wp = strchr(config, ':');
  if (!wp++) help();
  const char* bp = strchr(wp, ':');
  if (!bp++) help();

  size_t sets = atoi(std::string(config, wp).c_str());
  size_t ways = atoi(std::string(wp, bp).c_str());
  size_t linesz = atoi(bp);

  if (ways > 4 /* empirical */ && sets == 1)
    return new fa_cache_sim_t(ways, linesz, name);
  return new cache_sim_t(sets, ways, linesz, name);
}

void cache_sim_t::init()
{
  if (sets == 0 || (sets & (sets-1)))
    help();
  if (linesz < 8 || (linesz & (linesz-1)))
    help();

  idx_shift = 0;
  for (size_t x = linesz; x>1; x >>= 1)
    idx_shift++;

  tags = new uint64_t[sets*ways]();
  read_accesses = 0;
  read_misses = 0;
  bytes_read = 0;
  write_accesses = 0;
  write_misses = 0;
  bytes_written = 0;
  writebacks = 0;

  miss_handler = NULL;

  fifo_queues.clear();
  fifo_queues.resize(sets); 
}

cache_sim_t::cache_sim_t(const cache_sim_t& rhs)
 : sets(rhs.sets), ways(rhs.ways), linesz(rhs.linesz),
   idx_shift(rhs.idx_shift), name(rhs.name), log(false)
{
  tags = new uint64_t[sets*ways];
  memcpy(tags, rhs.tags, sets*ways*sizeof(uint64_t));
}

cache_sim_t::~cache_sim_t()
{
  print_stats();
  delete [] tags;
}

void cache_sim_t::print_stats()
{
  float mr = 100.0f*(read_misses+write_misses)/(read_accesses+write_accesses);

  std::cout << std::setprecision(3) << std::fixed;
  std::cout << name << " ";
  std::cout << "Bytes Read:            " << bytes_read << std::endl;
  std::cout << name << " ";
  std::cout << "Bytes Written:         " << bytes_written << std::endl;
  std::cout << name << " ";
  std::cout << "Read Accesses:         " << read_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Accesses:        " << write_accesses << std::endl;
  std::cout << name << " ";
  std::cout << "Read Misses:           " << read_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Write Misses:          " << write_misses << std::endl;
  std::cout << name << " ";
  std::cout << "Writebacks:            " << writebacks << std::endl;
  std::cout << name << " ";
  std::cout << "Miss Rate:             " << mr << '%' << std::endl;
}

uint64_t* cache_sim_t::check_tag(uint64_t addr)
{
  size_t idx = (addr >> idx_shift) & (sets-1);
  size_t tag = (addr >> idx_shift) | VALID;

  for (size_t i = 0; i < ways; i++)
    if (tag == (tags[idx*ways + i] & ~DIRTY))
      return &tags[idx*ways + i];

  return NULL;
}

//FIFO replacement policy code
uint64_t cache_sim_t::victimize(uint64_t addr)
{
    size_t my_set = (addr >> idx_shift) & (sets - 1);//算出這個資料對應到哪個set
    size_t target_slot = 0; //預設要填的way

    // ===Step 1:set還有空間可以直接放?===
    if (fifo_queues[my_set].size() < ways) {
      // 如果這個set還沒被塞滿（小於ways條slot），就可以直接找空位來用

      //準備一個布林陣列來紀錄哪些slot已經被佔用
      std::vector<bool> slot_used(ways, false);
      //複製一份queue來檢查現有有哪些已用slot
      std::queue<size_t> q_copy = fifo_queues[my_set];

      // 一個一個檢查queue的slot，把用過的標記起來
      while (!q_copy.empty()) {
          slot_used[q_copy.front()] = true;// queue裡的slot編號設為已用
          q_copy.pop();
      }

      // 從頭找一個還沒被佔用的 slot
      for (size_t slot = 0; slot < ways; ++slot) {
          if (!slot_used[slot]) {
              target_slot = slot;//記下這個 slot 當作新資料要填的地方
              break;
          }
      }
      // 把新 slot 記進 FIFO queue
      fifo_queues[my_set].push(target_slot);
    }
      
    // === Step 2:滿了，把最早進來的那個踢掉 ===
    else {
        target_slot = fifo_queues[my_set].front(); // 找到最老的 slot
        fifo_queues[my_set].pop(); // 把他踢掉
        fifo_queues[my_set].push(target_slot);  // 又把這個位置加回隊尾，循環利用
    }

    // === Step 3: 更新tag，把新資料寫進來 ===
    uint64_t evict_tag = tags[my_set * ways + target_slot]; // 備份舊資料
    tags[my_set * ways + target_slot] = (addr >> idx_shift) | VALID; // 寫新資料
    return evict_tag; // 回傳被踢掉的 tag（或沒被踢掉就回傳 0）
}
//FIFO replacement policy code END

// Original version of victimize function
/*
uint64_t cache_sim_t::victimize(uint64_t addr)
{
  // 計算 index：先右移 idx_shift 位元，取得 set index，再取模 sets
  size_t idx = (addr >> idx_shift) & (sets-1);
  // 使用 lfsr 產生隨機數，選擇要替換的 way
  size_t way = lfsr.next() % ways;
  // 找到要被替換掉的 cache line 的 tag
  uint64_t victim = tags[idx*ways + way];
  // 把目前這個位址的 tag 填進去對應位置，並標示為 VALID
  tags[idx*ways + way] = (addr >> idx_shift) | VALID;
  // 回傳被替換掉的 tag
  return victim;
}
*/

void cache_sim_t::access(uint64_t addr, size_t bytes, bool store)
{
  store ? write_accesses++ : read_accesses++;
  (store ? bytes_written : bytes_read) += bytes;

  uint64_t* hit_way = check_tag(addr);
  if (likely(hit_way != NULL))
  {
    if (store)
      *hit_way |= DIRTY;
    return;
  }

  store ? write_misses++ : read_misses++;
  if (log)
  {
    std::cerr << name << " "
              << (store ? "write" : "read") << " miss 0x"
              << std::hex << addr << std::endl;
  }

  uint64_t victim = victimize(addr);

  if ((victim & (VALID | DIRTY)) == (VALID | DIRTY))
  {
    uint64_t dirty_addr = (victim & ~(VALID | DIRTY)) << idx_shift;
    if (miss_handler)
      miss_handler->access(dirty_addr, linesz, true);
    writebacks++;
  }

  if (miss_handler)
    miss_handler->access(addr & ~(linesz-1), linesz, false);

  if (store)
    *check_tag(addr) |= DIRTY;
}

void cache_sim_t::clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval)
{
  uint64_t start_addr = addr & ~(linesz-1);
  uint64_t end_addr = (addr + bytes + linesz-1) & ~(linesz-1);
  uint64_t cur_addr = start_addr;
  while (cur_addr < end_addr) {
    uint64_t* hit_way = check_tag(cur_addr);
    if (likely(hit_way != NULL))
    {
      if (clean) {
        if (*hit_way & DIRTY) {
          writebacks++;
          *hit_way &= ~DIRTY;
        }
      }

      if (inval)
        *hit_way &= ~VALID;
    }
    cur_addr += linesz;
  }
  if (miss_handler)
    miss_handler->clean_invalidate(addr, bytes, clean, inval);
}

fa_cache_sim_t::fa_cache_sim_t(size_t ways, size_t linesz, const char* name)
  : cache_sim_t(1, ways, linesz, name)
{
}

uint64_t* fa_cache_sim_t::check_tag(uint64_t addr)
{
  auto it = tags.find(addr >> idx_shift);
  return it == tags.end() ? NULL : &it->second;
}

// Victimize function for fully associative cache
uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
    uint64_t evicted = 0;  // 用來存被淘汰掉的資料（預設為0，表示沒淘汰）
    uint64_t new_tag = (addr >> idx_shift);// 算出新要放進 cache 的 tag

    // Step 1: 如果 cache 已經塞滿（tags map 裡資料數量等於 ways 數）
    if (tags.size() == ways) {
        uint64_t oldest = fifo_queue.front(); // 找出 queue 最前面那個（最舊進來的）
        fifo_queue.pop(); // 將最舊的從 queue 移除

        // 在 tags map 找出這個 tag 並刪除，同時備份其內容
        auto found = tags.find(oldest);
        if (found != tags.end()) {
            evicted = found->second; // 備份被踢掉的 tag 內容
            tags.erase(found);  // 從 map 裡刪掉這個 tag
        }
    }

    // Step 2: 把新 tag 加進 FIFO queue 與 tags map
    fifo_queue.push(new_tag); // queue 尾端加新 tag（保持 FIFO）
    tags[new_tag] = new_tag | VALID;  // 將 tag 寫進 map 並設為有效
    return evicted;// 回傳被淘汰掉的內容（或0）
}
// Victimize function for fully associative cache END

//original version of victimize function
/*
uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
  uint64_t old_tag = 0;// 用來記錄被淘汰（要回傳）的舊 tag，預設為 0
  // 如果 cache 已經滿（已經存了 ways 條資料），要先淘汰一條
  if (tags.size() == ways)
  {
    auto it = tags.begin();// 取得 tags map 的第一個元素 iterator
    std::advance(it, lfsr.next() % ways);// 用 LFSR 亂數決定要淘汰哪個 tag（模擬隨機替換）
    old_tag = it->second;// 把被淘汰的 tag 內容記下來
    tags.erase(it); // 把這個 tag 從 map 中刪除
  }
  // 把新 tag（經過 idx_shift 處理，並加上 VALID）寫進 map
  tags[addr >> idx_shift] = (addr >> idx_shift) | VALID;
  // 回傳剛剛被淘汰的舊 tag（如果有淘汰就回傳，不然就是 0）
  return old_tag;
}
*/