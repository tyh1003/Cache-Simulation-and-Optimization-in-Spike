# Computer Organization  計算機組織 2025 Programming Assignment III

## 課程資訊
- **課程**: 成功大學資訊系 大二必修
- **學年**: 113學年度下學期
- **教授**: 涂嘉恒 (Tu, Chia-Heng)
- **作者**: MQ (GM 115)

## 作業說明
本次作業旨在透過有效運用快取（cache）來提升程式執行效能。現代運算系統利用快取將最近存取或經常使用的資料儲存在離處理器更近的位置，以降低記憶體存取延遲。在處理器設計的早期階段，設計者通常會根據應用程式的執行行為調整快取組織和配置。待處理器完成後，應用程式程式碼通常會優化以符合硬體特性、提升效能。本次作業將讓你練習這兩個概念。
-  **應用感知的快取設計（硬體角度）**
    -  實作管理有限快取資源的策略
    -  決定當快取滿時，哪些資料保留在快取中、哪些需要淘汰以騰出空間存放新資料
-  **快取架構感知的記憶體存取（軟體角度）**
    -  透過空間與時間區域性，組織程式碼以發揮快取效益
    -  安排記憶體存取模式以最大化快取命中率
在 Spike 模擬器中實作 FIFO（先進先出）快取替換政策，並且優化矩陣運算程式，以減少記憶體存取負擔。

### 作業概述  
#### Cache最佳化基本原理
-  硬體面向:(cache replacement policies)  
本作業:實作 FIFO 替換政策
    -  FIFO（First-In-First-Out，先進先出）：快取像一個 FIFO 佇列，最早進來的區塊會被最早淘汰，不考慮其他因素。
    -  LRU（Least Recently Used，最近最少使用）：淘汰最近最少被存取的快取區塊。
    -  LFU（Least-Frequently Used，最少使用次數）：淘汰存取次數最少的快取區塊。
-  軟體面向:(Algorithmic Level Method to Reduce Cache Miss) 
應用於演算法中，主要為improve the spatial or temporal locality  
    -  Loop Interchange（迴圈交換）：改變資料存取順序，以提升快取存取的空間區域性。
    -  Loop Fusion（迴圈合併）：將不同迴圈對同一陣列的操作合併，提升時間區域性。
    -  Loop Tiling（迴圈分塊/鋪磚）：將大型迴圈切分成較小的區塊（tiles），每個區塊內再組織計算，有助於重複利用快取內資料。
#### Spike 快取模擬
在 Spike 執行程式模擬時，執行指令（指令的位址）會輸入 L1 指令快取模擬器，而記憶體指令所需的資料（如存取記憶體時的資料）則會傳送到 L1 資料快取模擬器。這些快取模擬器在 Spike 中以 cache_sim_t 類別物件表示。快取模擬器的建立由 Spike 的命令列參數決定，例如 dc 旗標會建立 L1 資料快取模擬器，並可透過參數指定快取屬性（如組數與相連度）。更詳細資訊請參閱「程式實作部分」下「Exercise 1: Implement FIFO Data Cache Replacement Policy」。
cache_sim_t 類別定義了 Spike 建立的快取模擬器的行為，其原型在 riscv/cachesim.h 中定義、實作於 riscv/cachesim.cc。資料快取與指令快取都繼承自此類別所定義的介面。有幾個重要的成員函式用於操作快取，例如 access、check_tag、victimize。以下段落簡要介紹 access 與 check_tag 的高階概念。你應當追蹤原始碼以進一步理解這些成員函式的細節，了解 Spike 快取模擬器的運作機制與流程。
Spike 資料快取模擬流程重點如下：
當 Spike 模擬一個記憶體讀寫時，會啟動資料快取模擬，並呼叫 access 函式。其原型為 cache_sim_t::access(uint64_t addr, size_t bytes, bool store)，其中 addr 為要存取資料的位址，bytes 是資料長度，store 則代表這是讀還是寫操作。
至於 cache_sim_t::check_tag(uint64_t addr) 函式，它是用來檢查模擬資料快取中是否存在即將被存取的資料（也就是流程圖中的「check tag」步驟）。要存取資料所對應的快取組（set）索引，會透過 addr 參數用下列公式計算：
```
idx = (addr >> idx_shift) & (sets - 1)
```
如果你設定的是四組（four-set）資料快取，則 idx 值就是對 4 取餘數的結果，也就是 idx 的範圍是 0 到 3。接著，會將對應組（set）內所有 way 的 tag 與傳入的 addr 進行比對，以檢查快取中是否存在該資料。若有匹配，代表「快取命中（hit）」；否則就是「快取未命中（miss）」。
當發生快取未命中時，會呼叫 victimize 函式，找出一個受害（victim）快取區塊用來存放新資料。目前 Spike 只實作了隨機演算法（random algorithm），會隨機選擇一個受害區塊。
發生快取未命中時，必須選擇一個受害區塊來儲存新資料。特別地，Spike 利用「線性回饋移位暫存器」（LFSR, Linear-Feedback Shift Register）來實現偽隨機快取替換政策。

**注意**  
*電腦模擬不一定完全複製真實硬體的規格與行為，而是針對特定問題進行模擬，利用不同的工作負載與模擬設定，來評估不同設計之間的權衡。例如本次作業，我們聚焦於 L1 資料快取的設計，並試圖找出在指定工作負載下，能帶來最低資料快取未命中率的快取設計。此外，資料快取未命中所帶來的額外負擔（overhead）不會被考慮，而不同替換政策的實作負擔（如硬體面積、功耗、運算延遲）也未計算在內。*

#### 快取效能評估方法論（Performance Evaluation Methodology）
本方法論採用 Improvement Ratio 作為效能評估指標，量化不同程式版本或快取組態的效能提升幅度。核心機制透過記憶體存取週期數計算，結合快取命中/未命中統計數據進行分析。
##### 核心概念
**Improvement Ratio** 計算公式：
$$
\text{Improvement Ratio} = \frac{\text{MemCycle}_{\text{original}}}{\text{MemCycle}_{\text{improved}}}
$$
- $\text{MemCycle}_{\text{original}}$：原始版本總記憶體存取週期數
- $\text{MemCycle}_{\text{improved}}$：優化後版本總記憶體存取週期數
##### 記憶體存取開銷模型
總存取週期數計算公式：
```ini
MemCycle = cache_hits × latency_hit + cache_misses × penalty_miss
```
- `cache_hits`：快取命中次數
- `latency_hit`：單次命中延遲（預設 1 cycle）
- `cache_misses`：快取未命中次數
- `penalty_miss`：單次未命中懲罰（預設 100 cycles）
##### 實驗參數設定
| 參數類型 | 數值 | 說明 |
| :-- | :-- | :-- |
| 快取命中延遲 | 1 cycle | L1 cache存取時間 |
| 未命中懲罰 | 100 cycles | 主要記憶體存取時間 |
##### 範例計算
**原始版本數據**：
```bash
MemCycle_original = ((5,245,118 - 433,720) + (871,214 - 32,478)) × 1 
                  + (433,720 + 32,478) × 100 
                  = 52,269,934 cycles
```
**優化版本數據**：
```plaintext
MemCycle_improved = 15,150,540 cycles
```
**效能提升比**：
```python
Improvement_Ratio = 52,269,934 / 15,150,540 ≈ 3.45
```
##### 實作流程
1. **數據收集**：記錄程式執行時的 `cache_hits` 與 `cache_misses`
2. **週期數計算**：分別代入公式計算原始/優化版本的 `MemCycle`
3. **效能評估**：透過 Improvement Ratio 比較優化效果
##### 備註

- 本模型專注資料快取行為分析，未納入以下因素：
    - 主記憶體實際延遲波動
    - 快取替換策略的硬體開銷
- 適用於同架構下的相對效能比較
##### 參考數據模板
```plaintext
MemCycle_original = 52,269,934 cycles
MemCycle_improved = 15,150,540 cycles
Improvement Ratio = 3.45
```
### Assignment
**Programming Part (30%)**
-  Exercise 1 - Implement FIFO data cache replacement policy (10%)
-  Exercise 2 - Enhancement of software programs to reduce memory access overhead (20%)
    -  Exercise 2-1: Matrix Transpose (10%)
    -  Exercise 2-2: 2D Matrix Multiplication (10%)

**Demo Part (80%)**
-  Explain how your FIFO cache replacement policy works in Spike
-  Present the design philosophy of your optimized programs, explaining the techniques and concepts applied across all assignment sections

#### Exercise 1
- **整體流程說明**
  - 接收請求：當 CPU 要存取一個記憶體位址（addr），會呼叫 access()。
  - 更新存取計數：根據是讀還是寫，更新統計資訊（如 read_accesses、write_accesses 等）。
  - 查找 cache（check_tag）：
    - 根據地址計算 set（組）編號和 tag。
    - 在該 set 內檢查每個 way（槽、slot），看有沒有符合且有效的 tag。
    - 若有找到，稱為命中（hit）；若為寫入（store），就把該 tag 設 DIRTY、返回，流程結束。
    - 未命中（miss）：統計 miss；呼叫 victimize()，決定要取代（替換）哪一個 slot/way（根據 FIFO）。若被踢掉的 slot 同時有效且 DIRTY，需進行 writeback 到主記憶體（或下層 cache）。
    - 安裝新資料：將新 tag 寫入選中的 slot，更新 fifo_queues、若是寫操作，再設 DIRTY。
    - 如果有 miss handler（例如模擬 L2 cache），會遞交到下一層 cache 處理。

- **承接關係**
  - 呼叫 access() 時，會呼叫 check_tag() 檢查是否 hit。
  - miss 時呼叫 victimize() 決定替換目標 slot，並負責更新 tags 與 fifo_queues。
  - 如果有 writeback 需求，會通知 miss_handler。
  - 這些資訊與 cache simulator 的初始化（在 init()）有關，變數如 sets、ways、tags、fifo_queues 都會在 init() 先建好。

- **FIFO workflow and mechanism**
  - 計算set:根據位址計算這個資料屬於哪個set
  - 檢查set有沒有空位
  - 找到還沒用過的slot:使用布林array標記、從queue複製已用的slot、依序去找以追蹤資料先後順序
  - 如果set滿了，把queue最前面的slot找出來、並移到隊尾
  - 寫入新資料:備份tag、把新的tag寫入slot、加上VALID
  - 回傳tag:回傳原本的slot之tag、**writeback:寫回記憶體、確保資料不遺失**

- **fully associative cache FIFO workflow and mechanism**
 - 計算新tag:把傳入的位址 addr 右移 idx_shift 位數，得到新資料的 tag（new_tag）。
  - 檢查 cache 是否已滿:如果 cache 已經塞滿（tags.size() == ways），表示目前已經存滿 ways 條資料，必須淘汰一條舊資料。
  - FIFO 淘汰最舊資料
    - 從 FIFO queue 取出最前面（最舊進來）的 tag（oldest）。
    - 從 queue 移除這個 tag。
    - 在 tags map 中找出這個 tag，如果找得到，就備份它的內容，然後從 map 刪掉（實際完成淘汰動作）。
  - 安裝新資料
    - 把新 tag 加進 FIFO queue 的尾端，維持 FIFO 順序。
    - 把新 tag 寫進 tags map，並加上 VALID，表示有效。
  - 回傳被淘汰的 tag:回傳剛剛被淘汰掉的 tag（如果沒有淘汰就是 0），給外部做後續判斷（如是否 writeback）。

- **變數說明**
  - sets：
Cache 共有幾組（set），例如 64 組（通常為 2 的次方）。
  - ways：
每組 set 裡面有幾條 slot（又稱 way），例如 4-way set associative cache。
  - linesz：
每個 cache line（槽）的大小（byte），例如 64 bytes。
  - tags：
記錄所有 set、所有 slot 的 tag 資訊的陣列（一維，長度 = sets * ways）。
  - idx_shift：
block offset 的位數（用來從位址得到 set 編號和 tag）。
  - fifo_queues：
一個 vector，裡面每個元素都是一個 queue，紀錄每個 set 內 slot 的 FIFO 替換順序。
  - read_accesses, write_accesses：
讀寫次數統計。
  - read_misses, write_misses：
讀寫未命中（miss）次數統計。
  - writebacks：
cache line 被替換時需要寫回主記憶體的次數。
  - miss_handler：
指向下一層 cache 或記憶體的指標（可以呼叫下一層的 access 函數）。

##### cachesim.cc
###### cache_sim_t::victimize
```cpp
//FIFO replacement policy code
uint64_t cache_sim_t::victimize(uint64_t addr)
{
    size_t my_set = (addr >> idx_shift) & (sets - 1);//算出這個資料對應到哪個set
    size_t target_slot = 0; //預設要填的way

    // ===Step 1:set還有空間可以直接放?===
    if (fifo_order[my_set].size() < ways) {
      // 如果這個set還沒被塞滿（小於ways條slot），就可以直接找空位來用

      //準備一個布林陣列來紀錄哪些slot已經被佔用
      std::vector<bool> slot_used(ways, false);
      //複製一份queue來檢查現有有哪些已用slot
      std::queue<size_t> q_copy = fifo_order[my_set];

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
      fifo_order[my_set].push(target_slot);
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
  size_t idx = (addr >> idx_shift) & (sets-1);
  size_t way = lfsr.next() % ways;
  uint64_t victim = tags[idx*ways + way];
  tags[idx*ways + way] = (addr >> idx_shift) | VALID;
  return victim;
}
*/
```
###### fa_cache_sim_t
```cpp
// Victimize function for fully associative cache
uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
    uint64_t evicted = 0;  // 用來存被淘汰掉的資料（預設為0，表示沒淘汰）
    uint64_t new_tag = (addr >> idx_shift);// 算出新要放進 cache 的 tag

    // Step 1: 如果 cache 已經塞滿（tags map 裡資料數量等於 ways 數）
    if (tags.size() == ways) {
        uint64_t oldest = fifo_tag_queue.front(); // 找出 queue 最前面那個（最舊進來的）
        fifo_tag_queue.pop(); // 將最舊的從 queue 移除

        // 在 tags map 找出這個 tag 並刪除，同時備份其內容
        auto found = tags.find(oldest);
        if (found != tags.end()) {
            evicted = found->second; // 備份被踢掉的 tag 內容
            tags.erase(found);  // 從 map 裡刪掉這個 tag
        }
    }

    // Step 2: 把新 tag 加進 FIFO queue 與 tags map
    fifo_tag_queue.push(new_tag); // queue 尾端加新 tag（保持 FIFO）
    tags[new_tag] = new_tag | VALID;  // 將 tag 寫進 map 並設為有效
    return evicted;// 回傳被淘汰掉的內容（或0）
}
// Victimize function for fully associative cache END

//original version of victimize function
/*
uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
  uint64_t old_tag = 0;
  if (tags.size() == ways)
  {
    auto it = tags.begin();
    std::advance(it, lfsr.next() % ways);
    old_tag = it->second;
    tags.erase(it);
  }
  tags[addr >> idx_shift] = (addr >> idx_shift) | VALID;
  return old_tag;
}
*/
```
###### 完整版cachesim.cc
```cpp
// See LICENSE for license details.

#include "cachesim.h"
#include "common.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>

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

uint64_t cache_sim_t::victimize(uint64_t addr)
{
  size_t idx = (addr >> idx_shift) & (sets-1);
  size_t way = lfsr.next() % ways;
  uint64_t victim = tags[idx*ways + way];
  tags[idx*ways + way] = (addr >> idx_shift) | VALID;
  return victim;
}

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

uint64_t fa_cache_sim_t::victimize(uint64_t addr)
{
  uint64_t old_tag = 0;
  if (tags.size() == ways)
  {
    auto it = tags.begin();
    std::advance(it, lfsr.next() % ways);
    old_tag = it->second;
    tags.erase(it);
  }
  tags[addr >> idx_shift] = (addr >> idx_shift) | VALID;
  return old_tag;
}
```
##### cachesim.h
- **queue**
  - 作業講義提示使用
  - queue是FIFO的資料結構，最早進來的先被移走，新進來的排在隊伍最後面。
- **std::queue<uint64_t> fifo_queue;**
  - 在 .h 檔裡用 C++ 標準函式庫的 queue，來建立一個存放 uint64_t(tag值)的 FIFO 佇列。
  - 每次 cache 加新資料進來時，就用 fifo_queue.push(tag) 把 tag 放到隊尾。
  - 當 cache 滿時，要淘汰舊資料，就用 fifo_queue.front() 取得最早進來的 tag，再用 fifo_queue.pop() 把這個最舊的 tag 移除
- **相關函式**
  - .push():把新資料加到隊伍尾端
  - .pop():把最前面的資料移除
  - .front():取得最前面的資料
  - .empty():判斷是否是空的
```h
// See LICENSE for license details.

#ifndef _RISCV_CACHE_SIM_H
#define _RISCV_CACHE_SIM_H

#include "memtracer.h"
#include "common.h"
#include <cstring>
#include <string>
#include <map>
#include <cstdint>
//////////////////////////////////////////////////////////////////////
#include <queue>
//////////////////////////////////////////////////////////////////////
class lfsr_t
{
 public:
  lfsr_t() : reg(1) {}
  lfsr_t(const lfsr_t& lfsr) : reg(lfsr.reg) {}
  uint32_t next() { return reg = (reg>>1)^(-(reg&1) & 0xd0000001); }
 private:
  uint32_t reg;
};

class cache_sim_t
{
 public:
  cache_sim_t(size_t sets, size_t ways, size_t linesz, const char* name);
  cache_sim_t(const cache_sim_t& rhs);
  virtual ~cache_sim_t();

  void access(uint64_t addr, size_t bytes, bool store);
  void clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval);
  void print_stats();
  void set_miss_handler(cache_sim_t* mh) { miss_handler = mh; }
  void set_log(bool _log) { log = _log; }

  static cache_sim_t* construct(const char* config, const char* name);

 protected:
  static const uint64_t VALID = 1ULL << 63;
  static const uint64_t DIRTY = 1ULL << 62;

  virtual uint64_t* check_tag(uint64_t addr);
  virtual uint64_t victimize(uint64_t addr);

  lfsr_t lfsr;
  cache_sim_t* miss_handler;

  size_t sets;
  size_t ways;
  size_t linesz;
  size_t idx_shift;

  uint64_t* tags;
  
  uint64_t read_accesses;
  uint64_t read_misses;
  uint64_t bytes_read;
  uint64_t write_accesses;
  uint64_t write_misses;
  uint64_t bytes_written;
  uint64_t writebacks;

  std::string name;
  std::vector<std::queue<size_t>> fifo_queues; // FIFO queues for each set
  bool log;

  void init();
};

class fa_cache_sim_t : public cache_sim_t
{
 public:
  fa_cache_sim_t(size_t ways, size_t linesz, const char* name);
  uint64_t* check_tag(uint64_t addr);
  uint64_t victimize(uint64_t addr);
 private:
  static bool cmp(uint64_t a, uint64_t b);
  std::map<uint64_t, uint64_t> tags;

  /////////////////////////////////
  std::queue<uint64_t> fifo_queue;
  /////////////////////////////////
};

class cache_memtracer_t : public memtracer_t
{
 public:
  cache_memtracer_t(const char* config, const char* name)
  {
    cache = cache_sim_t::construct(config, name);
  }
  ~cache_memtracer_t()
  {
    delete cache;
  }
  void set_miss_handler(cache_sim_t* mh)
  {
    cache->set_miss_handler(mh);
  }
  void clean_invalidate(uint64_t addr, size_t bytes, bool clean, bool inval)
  {
    cache->clean_invalidate(addr, bytes, clean, inval);
  }
  void set_log(bool log)
  {
    cache->set_log(log);
  }
  void print_stats()
  {
    cache->print_stats();
  }

 protected:
  cache_sim_t* cache;
};

class icache_sim_t : public cache_memtracer_t
{
 public:
  icache_sim_t(const char* config, const char* name = "I$")
	  : cache_memtracer_t(config, name) {}
  bool interested_in_range(uint64_t UNUSED begin, uint64_t UNUSED end, access_type type)
  {
    return type == FETCH;
  }
  void trace(uint64_t addr, size_t bytes, access_type type)
  {
    if (type == FETCH) cache->access(addr, bytes, false);
  }
};

class dcache_sim_t : public cache_memtracer_t
{
 public:
  dcache_sim_t(const char* config, const char* name = "D$")
	  : cache_memtracer_t(config, name) {}
  bool interested_in_range(uint64_t UNUSED begin, uint64_t UNUSED end, access_type type)
  {
    return type == LOAD || type == STORE;
  }
  void trace(uint64_t addr, size_t bytes, access_type type)
  {
    if (type == LOAD || type == STORE) cache->access(addr, bytes, type == STORE);
  }
};

#endif
```
#### Exercise 2 
##### Exercise2-1
- **矩陣轉置演算法概念**
一般矩陣轉置時，會直接row、column 一個一個處理，每次都從不同 row、不同 column 取值，這會導致 cache 命中率很低，因為記憶體存取模式跟 cache 的分佈不一致。因此，把整個矩陣分割成很多小區塊（block），每次只處理一個 block，讓這個 block 內部的資料比較有機會都被 cache 一次載入後重複利用，大幅提升cache hit rate，減少memory access overhead。

- **為何是BLOCK SIZE=8**
block size的選擇跟cache結構有關
  - **基本資訊**:cache line=32 bytes、int=4 bytes，一個32-byte cache line可以放**8個int型態資料**。
  - 一個8*8 block有64個元素，佔256bytes，**block size 選擇要小於等於 cache 能一次裝的資料量**
  - block size太大:超過 cache 能一次裝載的量
  - block size太小:cache locality 利用不佳
  - **BLOCK SIZE=8，對齊cache line**

```c
#define BLOCK_SIZE 8 //定義BLOCK_SIZE為8，區塊大小，有助於提升記憶體存取效率

void matrix_transpose(int n, int *dst, int *src) { //宣告matrix_transpose 的函數(n=矩陣長度、n*n正方形矩陣；dst=目標陣列位置；src=來源陣列位置)
    // Inplement your code here
    for (int i = 0; i < n; i += BLOCK_SIZE) { //以BLOCK_SIZE為單位，由上到下分區處理，矩陣row，i~n(以BLOCK_SIZE為單位)
        for (int j = 0; j < n; j += BLOCK_SIZE) {//以BLOCK_SIZE為單位，由左到右分區處理，矩陣column，j~n(以BLOCK_SIZE為單位)
            ///////處理一個block//////
            for (int x = i; x < i + BLOCK_SIZE && x < n; x++) { //在目前的block，由row逐一處理(x: i ~ min(i+BLOCK_SIZE,n)-1)
                for (int y = j; y < j + BLOCK_SIZE && y < n; y++) { //在目前的block，由column逐一處理(y: j ~ min(j+BLOCK_SIZE,n)-1)
                    dst[y + x * n] = src[x + y * n]; 
                    //src[x+y*n]：取得原始矩陣第 x row、第 y column 的元素
                    //row與column對調
                    //dst[y+x*n]：寫到轉置後的位置
                }
            }
        }
    }
}
```

##### Exercise2-2
- **矩陣乘法演算法概念**
傳統的三層for迴圈，資料存取的區域性很差，尤其是大矩陣時，很容易造成cache miss，效率低下。使用這個演算法把三個維度（row、column、k）都分成小block，每次只計算一個小block內的乘加，這樣一來：每個block運算時，a、b兩個矩陣裡的小區塊資料可以被cache載入後重複使用，cache locality變好；記憶體存取更連續，減少cache miss。

- **BLOCK SIZE=8**
block size一樣設成8，是因為這樣能配合cache line（32 bytes），剛好放下多個int，達到cache最有效利用。

```c
#define BLOCK_SIZE 8 //定義BLOCK_SIZE為8，區塊大小，有助於提升記憶體存取效率

void matrix_multiply(int *a, int *b, int *output, int i, int k, int j) { //宣告matrix_multiply函數，a:左矩陣(i*k)，b:右矩陣(k*j)，output:結果(i*j)
    // Inplement your code here
    for (int i0 = 0; i0 < i; i0 += BLOCK_SIZE) { //以BLOCK_SIZE為單位，row分區(i方向)
        for (int j0 = 0; j0 < j; j0 += BLOCK_SIZE) { //以BLOCK_SIZE為單位，column分區(j方向)
            for (int k0 = 0; k0 < k; k0 += BLOCK_SIZE) { //以BLOCK_SIZE為單位，中間維度分區(k方向)
                /////處理一個block/////
                for (int ii = i0; ii < i0 + BLOCK_SIZE && ii < i; ii++) { //在目前block的row內逐一處理(ii: i0 ~ min(i0+BLOCK_SIZE, i)-1)
                    for (int jj = j0; jj < j0 + BLOCK_SIZE && jj < j; jj++) { //在目前block的column內逐一處理(jj: j0 ~ min(j0+BLOCK_SIZE, j)-1)
                        int sum = output[ii * j + jj]; //可設0，若output已初始化
                        for (int kk = k0; kk < k0 + BLOCK_SIZE && kk < k; kk++) { //在目前block的k方向逐一處理(kk: k0 ~ min(k0+BLOCK_SIZE, k)-1)
                            sum += a[ii * k + kk] * b[kk * j + jj]; //a(ii,kk) * b(kk,jj)加總
                        }
                        output[ii * j + jj] = sum; //寫回output
                    }
                }
            }
        }
    }
}
```
### Test Your Assignment
Test my code of whole exercise 2 implementation
```
make check
```
#### random
**BLOCK SIZE=8**
CompOrg@CompOrg2025:/media/sf_HW/CO2025HW3/CO_StudentID_HW3/exercise2$ make check  
=============== Exercise 2-1 ===============  
Original version  
D$ Bytes Read:            102260716  
D$ Bytes Written:         49349062  
D$ Read Accesses:         21310184  
D$ Write Accesses:        8675596  
D$ Read Misses:           1139243  
D$ Write Misses:          709271  
D$ Writebacks:            763429  
D$ Miss Rate:             6.165%  

Memory subsystem access overhead =  212988666 (cpu cycle)  

Improved version  
D$ Bytes Read:            114564728  
D$ Bytes Written:         50448174  
D$ Read Accesses:         24392855  
D$ Write Accesses:        8952616  
D$ Read Misses:           242573  
D$ Write Misses:          689483  
D$ Writebacks:            717374  
D$ Miss Rate:             2.795%  

Memory subsystem access overhead =  125619015 (cpu cycle)  

Improved ratio:  **1.6955129444375918 > 1.6**  
Output Correctness:  **Pass**  

=============== Exercise 2-2 ===============   
Original version  
D$ Bytes Read:            66317076  
D$ Bytes Written:         9310602  
D$ Read Accesses:         14424911  
D$ Write Accesses:        2215985  
D$ Read Misses:           1263655  
D$ Write Misses:          28348  
D$ Writebacks:            67173  
D$ Miss Rate:             7.764%  

Memory subsystem access overhead =  144549193 (cpu cycle)

Improved version
D$ Bytes Read:            84963996  
D$ Bytes Written:         11383826  
D$ Read Accesses:         18836641  
D$ Write Accesses:        2734135  
D$ Read Misses:           124147  
D$ Write Misses:          25129  
D$ Writebacks:            50305  
D$ Miss Rate:             0.692%  

Memory subsystem access overhead =  36349100 (cpu cycle)  

Improved ratio:  **3.9766924903230065 > 3**  
Output Correctness:  **Pass**  

#### FIFO
**BLOCK SIZE=8**  
CompOrg@CompOrg2025:/media/sf_HW/CO2025HW3/CO_StudentID_HW3/exercise2$ make check  
=============== Exercise 2-1 ===============  
Original version  
D$ Bytes Read:            102260716  
D$ Bytes Written:         49349062  
D$ Read Accesses:         21310184  
D$ Write Accesses:        8675596  
D$ Read Misses:           1132459  
D$ Write Misses:          674845  
D$ Writebacks:            726049  
D$ Miss Rate:             6.027%  

Memory subsystem access overhead =  208908876 (cpu cycle)  

Improved version  
D$ Bytes Read:            114564728  
D$ Bytes Written:         50448174  
D$ Read Accesses:         24392855  
D$ Write Accesses:        8952616  
D$ Read Misses:           203675  
D$ Write Misses:          674298  
D$ Writebacks:            698595  
D$ Miss Rate:             2.633%  

Memory subsystem access overhead =  120264798 (cpu cycle)  

Improved ratio:  **1.73707418524912 > 1.6**  
Output Correctness:  **Pass**  

=============== Exercise 2-2 ===============  
Original version  
D$ Bytes Read:            66317076  
D$ Bytes Written:         9310602  
D$ Read Accesses:         14424911  
D$ Write Accesses:        2215985  
D$ Read Misses:           1242681  
D$ Write Misses:          28261  
D$ Writebacks:            65873  
D$ Miss Rate:             7.637%  

Memory subsystem access overhead =  142464154 (cpu cycle)  

Improved version  
D$ Bytes Read:            84963996  
D$ Bytes Written:         11383826  
D$ Read Accesses:         18836641  
D$ Write Accesses:        2734135  
D$ Read Misses:           98056  
D$ Write Misses:          20154  
D$ Writebacks:            47863  
D$ Miss Rate:             0.548%  

Memory subsystem access overhead =  33273566 (cpu cycle)  

Improved ratio:  **4.2816016173319085 > 3**  
Output Correctness:  **Pass**  

#### 測試成果比較
最佳區塊大小 (T) = 快取線大小 (CB) / 每個元素的字節數 (SEW)  

| 測試項目 | 優化說明 | 演算法 | 失誤率 (原始 -> 優化) | 效能提升率 (Improved Ratio) |
| :-- | :-- | :-- | :-- | :-- |
| **測試二** | 隨機亂數測試 (Block size=8) | 矩陣轉置 | 6.165% -> 2.795% | **1.696** |
|  |  | 矩陣相乘 | 7.764% -> 0.692% | **3.977** |
| **測試四** | 自訂 FIFO (Block size=8) | 矩陣轉置 | 6.027% -> 2.633% | **1.737** |
|  |  | 矩陣相乘 | 7.637% -> **0.548%** | **4.282** |
| **測試五** | 自訂 FIFO (Block size=4) | 矩陣轉置 | 6.027% -> 2.890% | **1.547** |
|  |  | 矩陣相乘 | 7.637% -> 0.576% | **3.666** |
| **測試六** | 自訂 FIFO (Block size=16) | 矩陣轉置 | 6.027% -> 2.772% | **1.710** |
|  |  | 矩陣相乘 | 7.637% -> 1.297% | **3.101** |

### For DEMO

#### 1. Describe the workflow and mechanism in Spike, related to cache simulation. (20%)
在 Spike 的 cache simulator，整體流程如下：
- 存取流程 access()
當 CPU 要讀寫某個記憶體位址時，會呼叫 access()，這時會先計算對應的 set 和 tag，再進入 cache 查找。
- cache 查找 check_tag()
根據地址計算出 set 編號和 tag，然後檢查這個 set 裡所有 slot（way），看有沒有有效且 tag 相同的資料。如果有就是 hit，沒有就是 miss。
- miss 時的替換 victimize()
如果沒有命中，就需要替換一個 slot。Spike 採用**FIFO（先進先出）**的替換策略。每個 set 都有一個 queue，記錄 slot 被填入的順序。
  - 如果 set 內還有空位，就找還沒用過的 slot。
  - 如果滿了，就把 queue 最前面的 slot（最舊的資料）踢掉，再把新資料寫進這個位置，並把這個 slot 編號加回隊尾。
  - 被踢掉且有 dirty bit 的資料，會觸發 writeback，寫回主記憶體。
- 統計資訊與變數
Spike 會統計所有的讀寫次數、miss 次數和 writeback 次數等資訊，最後可以輸出統計結果。
- 延伸機制
若有 miss handler，例如模擬 L2 cache，miss 時會自動把請求遞交給下一層 cache 處理。

**總結：**
Spike cache simulator 主要流程是先查 tag，沒命中時用 FIFO 決定要替換哪個 slot，每組 set 都有一個 FIFO queue 管理 slot 替換順序，確保最早進來的資料最早被淘汰。所有讀寫、miss、writeback 都會被記錄統計。

#### 2. Describe the concept behind your modified matrix transpose algorithm. (20%)
- 修改過的矩陣轉置演算法主要採用了**區塊化（blocking/tiling）**的技術，目的是提升記憶體存取效率，減少cache miss，從而提升整體效能。
- 傳統的矩陣轉置通常是直接用兩層for迴圈，逐一存取每個row與column。這種寫法會導致每次存取的記憶體區塊在空間上跳動很大，很難充分利用cache的區域性（locality），因此cache hit rate偏低，效能也不佳。
- 區塊化的做法，是把整個矩陣切成多個小區塊（block），例如本題選用BLOCK_SIZE=8，即每次處理8x8的子矩陣。演算法會先用外層for迴圈走訪整個矩陣的區塊，再在每個block裡面用兩層內迴圈完成轉置。這樣設計讓每次操作時，block內的資料可以盡可能一起被cache載入，也能被重複利用，提高cache的命中率，降低memory access overhead。
- 為什麼選BLOCK_SIZE=8，是因為本題的cache line大小是32 bytes，而int型態4 bytes，一個cache line可以放8個int。這樣設計block size正好對齊cache line，讓每次存取都能最大化利用cache空間，降低cache line loading的浪費。如果block size太大會超過cache能裝的容量，導致資料被過早替換；太小則無法充分發揮locality。所以8這個數字在這個cache結構下是比較合適的選擇。

**總結:**
我的區塊化轉置演算法，結合了cache架構特性與資料區域性的原理，大幅提升了cache hit rate和整體效能，從測試結果也可以看到miss rate和memory overhead都有明顯降低，效能提升近2倍。

#### 3. Describe the concept behind your modified matrix multiplication algorithm. (20%)
- 矩陣乘法演算法同樣採用了**區塊化（blocking/tiling）**技術來最佳化效能。
- 傳統三層for迴圈的矩陣相乘，運算時a、b、output三個矩陣的資料存取區域性都很差，特別是資料跨row、跨column時，cache命中率會大幅下降，這對大矩陣來說影響很大，會讓cache miss大增，導致效能低下。
- 將三個維度（row、column、k）都以block方式分割，三層外迴圈分別以BLOCK_SIZE=8為步進，內部再用三層for處理一個小block的所有乘加運算。這樣做的好處是：每一個小block計算時，a和b兩個矩陣的小區塊資料會多次重複被訪問，這些資料能夠暫存在cache中，大幅提升cache hit率，同時讓output block的寫入也能更有效率地對齊cache line。
- block size一樣設定成8，是因為每個cache line 32 bytes，一次可以裝8個int，這讓資料排列更容易對齊cache，避免cache line頻繁替換。根據測試結果，使用block size=8時，cache miss rate和memory access overhead明顯下降，效能提升了將近4倍。
- 如果block size過小，雖然能提高locality，但無法充分利用cache空間；如果太大，資料又容易在還沒被用完前就被替換掉。

**總結:**
這個區塊化矩陣乘法演算法，是根據cache硬體結構設計的最佳化策略，利用小區塊多次重用來提升locality，降低cache miss，提升整體運算效能，實驗數據也證實了這種設計大幅優於傳統寫法。

#### 4. Describe the concept behind your design philosophy of previous Assignment I and Assignment II. (20%)
##### Assignment I
- 第一題Bubble Sort
拆解出每個步驟：計算元素位址、資料讀取、比較與交換，然後以直譯方式用RISC-V指令分別實現。透過暫存器合理分配，確保每一步都與C原始語意完全對應，並保持迴圈與if條件判斷結構清楚。
- 第二題Array Search
強調流程的明確性：初始化、逐項比較、比對成功即break，並用分支與暫存器來處理索引及條件跳躍。特別強調 early break 提升效率，不需要多餘運算，並將結果直接儲存回memory，確保正確性。
- 第三題Linked-List Merge Sort
將複雜的資料結構操作分為三大區塊：split、merge、traverse。每一部分都根據C的語意，利用快慢指標、條件分支、記憶體存取指令精確處理。合併階段特別處理鏈結串列頭尾情形，保證不論節點數量、分布，都能正確排序並重建list。

**總結:**
- 強調每一步驟與C語言語意一致，降低debug難度
- 暫存器與memory操作盡量清楚、直接，不留多餘或難懂的指令
- 流程以可讀性和正確性為第一考量，兼顧效率與模組化

##### Assignment II
- 流程與演算法先行，逐步映射至組合語言：
我先理解每個問題的核心流程，例如FFT就是分為複數加減、旋轉因子運算、bit-reverse排序三大區塊。對於array multiply則是逐元素運算、向量化與積累。先把C語言或數學步驟拆清楚，再逐步對映到RISC-V指令，確保邏輯上每個步驟都能以組語精確落實，不會漏失重要細節。
- 針對硬體資源（暫存器、資料型態、記憶體）最佳化：
像FFT中的複數加減，我直接對應到浮點運算指令，先將實部/虛部分別載入暫存器，再進行運算，避免重複讀寫memory，減少資料搬移。陣列運算時，盡量將常用的資料（如當前index、累積值、陣列基址）分配到暫存器中，減少記憶體存取次數，提高指令效率。
- 善用ISA特色與新技術（如RISC-V Vector Extension）：
針對Q2-2與Q3的向量化題目，我仔細學習RVV的用法，掌握vsetvli/vle/vse等指令，讓同一份運算用SIMD方式批次處理，大幅提升平行效率。傳統RV64G題目則重視每個指令的資料流與分支，確保控制流程嚴謹且能正確 early break 或累積結果。

**總結：**
整體來說，我設計時強調結構清楚、效率優先與可讀性，每一段code都可以追溯到C語言或數學流程，並善用暫存器和向量指令提升效率，同時注意結果正確性與系統效能分析。這不僅讓我的程式通過所有測資，也讓我能精確分析每一段運算的效率瓶頸與優勢。

### 結語
#### 開發筆記
這次作業壓力最大的地方是DEMO，怕有問答，所以一直很有壓力也很努力的想去弄懂，其次時間不太夠，所以感覺整體零零散散的QQ。  
一開始使用G教授去寫，都失敗了；後來用C教授的有成功、也能夠debug。
花了很久搞懂要怎麼測試這兩題、也花很多時間在理解自己到底是在動甚麼東西。
寫程式的部分，原本覺得第二題比較好寫、程式碼上也好理解，但是block size的值好難懂啊啊啊!!!之後第一題也是C教授輔助寫的，結果概念上似乎更簡單了。
測試的部分反覆遇到虛擬機爆炸、vscode爆炸的問題，目前是重裝虛擬機以及反覆刪vscode的快取==

#### 心得-初學者如何做這題
首先，我是一個硬體沒啥搞懂就來修祭祖的人，遇到這次的大刀，真的很季。
第一題非常硬體銜接，建議把說明書看個三次、配合各種AI工具詳細說明，第一題應該看懂、prompt下對、搞懂程式碼在幹嘛就行了。
第二題基本上就是把L5讀懂，懂到能融會貫通、還要清楚各個名詞在幹嘛就行了(我自己不太懂各個名詞、所以卡卡的)，做法就是切block多幾個迴圈讓他跑而已、不懂值要設多大也可以用試的。
OK，這學期的作業總算到一段落，真的要暴斃了==
祝自己一切順利，至少到6月底要好好撐下去、教授不要當我，我真的很努力了啦嗚嗚嗚。
06/09 by MQ


