#pragma once

#include "page.h"
#include <string>
#include <fstream>
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace sqlcc {

/**
 * @brief 磁盘管理器类，负责页面的磁盘I/O操作
 * 
 * Why: 数据库系统需要持久化存储数据，磁盘管理器负责处理所有与磁盘的交互，
 *      包括页面的读取、写入和分配，是数据库存储引擎的基础组件。
 *      对于不了解数据库实现的学生来说，磁盘管理器展示了如何将内存中的数据结构
 *      持久化到磁盘，以及如何管理磁盘空间。
 * 
 * What: DiskManager类提供了数据库页面的磁盘I/O操作，包括：
 *       1. 将页面从内存写入磁盘（WritePage）
 *       2. 从磁盘读取页面到内存（ReadPage）
 *       3. 分配新的页面空间（AllocatePage）
 *       4. 获取数据库文件大小（GetFileSize）
 *       这些操作构成了数据库持久化的基础。
 * 
 * How: DiskManager使用C++的fstream库进行文件I/O操作，维护一个文件流对象
 *      和文件大小信息，通过计算页面偏移量来定位磁盘上的页面位置。
 *      每个页面有固定大小，页面ID乘以页面大小即可得到页面在文件中的偏移量。
 *      磁盘管理器还维护一个页面计数器，用于分配新的页面ID。
 */
class DiskManager {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要初始化磁盘管理器，打开数据库文件并准备进行I/O操作。
     *      对于不了解数据库实现的学生来说，构造函数展示了如何初始化
     *      文件I/O资源，以及如何检查文件是否存在。
     * 
     * What: 构造函数接收数据库文件路径作为参数，打开文件流，初始化文件大小
     *       和页面计数器，为后续的I/O操作做准备。
     * 
     * How: 使用fstream打开文件，如果文件不存在则创建新文件。
     *       通过seekg和tellg获取文件大小，初始化next_page_id_为0。
     *       如果文件已存在且大小大于0，则计算现有页面数量。
     * 
     * @param db_file 数据库文件路径
     */
    explicit DiskManager(const std::string& db_file);

    /**
     * @brief 析构函数
     * 
     * Why: 需要释放文件流资源，确保文件正确关闭，防止数据丢失。
     *      对于不了解数据库实现的学生来说，析构函数展示了如何正确
     *      清理文件I/O资源，确保数据完整性。
     * 
     * What: 析构函数负责关闭数据库文件流，释放系统资源。
     * 
     * How: 如果文件流处于打开状态，则调用close方法关闭文件。
     *       fstream的析构函数会自动关闭文件，但显式关闭更安全。
     */
    ~DiskManager();

    /**
     * @brief 禁止拷贝构造
     * 
     * Why: 磁盘管理器管理文件流资源，不应该被拷贝，以防止多个对象
     *      同时操作同一个文件流，导致数据混乱或资源泄漏。
     *      对于不了解数据库实现的学生来说，这展示了资源管理类的设计原则。
     * 
     * What: 通过将拷贝构造函数声明为delete，禁止编译器生成默认的拷贝构造函数。
     * 
     * How: 使用C++11的= delete语法显式禁用拷贝构造函数。
     */
    DiskManager(const DiskManager&) = delete;

    /**
     * @brief 禁止赋值操作
     * 
     * Why: 磁盘管理器管理文件流资源，不应该被赋值，以防止多个对象
     *      同时操作同一个文件流，导致数据混乱或资源泄漏。
     *      对于不了解数据库实现的学生来说，这展示了资源管理类的设计原则。
     * 
     * What: 通过将赋值操作符声明为delete，禁止编译器生成默认的赋值操作符。
     * 
     * How: 使用C++11的= delete语法显式禁用赋值操作符。
     */
    DiskManager& operator=(const DiskManager&) = delete;

    /**
     * @brief 将页面写入磁盘
     * 
     * Why: 数据库需要将修改后的页面持久化到磁盘，以保证数据的持久性和一致性。
     *      对于不了解数据库实现的学生来说，这个方法展示了如何将内存中的
     *      数据结构写入到磁盘文件的特定位置。
     * 
     * What: WritePage方法接收一个页面对象，将其内容写入到磁盘文件的对应位置。
     *       页面ID决定了页面在文件中的位置，页面内容包含了实际的数据。
     * 
     * How: 计算页面在文件中的偏移量（页面ID * 页面大小），使用seekp定位到
     *      该位置，然后调用write方法写入页面数据。写入后返回true表示成功。
     *      如果发生错误，则返回false。
     * 
     * @param page 要写入的页面
     * @return 写入成功返回true，否则返回false
     */
    bool WritePage(const Page& page);

    /**
     * @brief 从磁盘读取页面
     * 
     * Why: 当缓冲池需要加载不在内存中的页面时，需要从磁盘读取页面数据。
     *      对于不了解数据库实现的学生来说，这个方法展示了如何从磁盘文件的
     *      特定位置读取数据到内存中的数据结构。
     * 
     * What: ReadPage方法接收一个页面ID和一个页面对象指针，从磁盘文件中
     *       读取对应页面的数据，并填充到传入的页面对象中。
     * 
     * How: 计算页面在文件中的偏移量（页面ID * 页面大小），使用seekg定位到
     *      该位置，然后调用read方法读取页面数据到页面对象中。读取成功返回true。
     *      如果发生错误或页面ID超出文件范围，则返回false。
     * 
     * @param page_id 页面ID
     * @param page 用于存储读取数据的页面对象
     * @return 读取成功返回true，否则返回false
     */
    bool ReadPage(int32_t page_id, Page* page);
    
    /**
     * @brief 批量从磁盘读取多个页面，优化连续访问性能
     * 
     * Why: 批量读取可以减少I/O操作次数和系统调用开销，提高磁盘访问效率
     * What: BatchReadPages方法一次性读取多个页面，对连续页面进行优化排序
     * How: 按页面ID排序以优化磁盘访问，使用预读机制提高性能
     * 
     * @param page_ids 页面ID数组，指定要读取的页面
     * @param data_buffers 数据缓冲区数组，存储读取的页面数据
     * @param count 页面数量
     * 
     * @return 成功读取的页面数量
     */
    size_t BatchReadPages(const std::vector<int32_t>& page_ids, std::vector<char*>& data_buffers);
    
    /**
     * @brief 预读指定页面到操作系统缓存
     * 
     * Why: 预读可以提前将页面加载到操作系统缓存，减少未来的I/O等待时间
     * What: PrefetchPage方法使用操作系统提供的预读机制加载页面
     * How: 使用posix_fadvise或readahead系统调用实现预读
     * 
     * @param page_id 页面ID，指定要预读的页面
     * 
     * @return 预读成功返回true，否则返回false
     */
    bool PrefetchPage(int32_t page_id);
    
    /**
     * @brief 批量预读多个页面到操作系统缓存
     * 
     * Why: 批量预读可以进一步减少I/O开销，特别适用于顺序访问模式
     * What: BatchPrefetchPages方法一次性预读多个页面，优化磁盘访问模式
     * How: 按页面ID排序以优化磁盘访问，使用预读机制提高性能
     * 
     * @param page_ids 页面ID数组，指定要预读的页面
     * 
     * @return 成功预读的页面数量
     */
    size_t BatchPrefetchPages(const std::vector<int32_t>& page_ids);

    /**
     * @brief 分配新页面，返回页面ID
     * 
     * Why: 当数据库需要存储新数据时，需要分配新的页面空间。
     *      对于不了解数据库实现的学生来说，这个方法展示了如何管理
     *      磁盘空间的分配，以及如何生成唯一的页面标识符。
     * 
     * What: AllocatePage方法分配一个新的页面ID，并扩展数据库文件大小
     *       以容纳新页面。返回的页面ID可以用于后续的读写操作。
     * 
     * How: 使用原子递增next_page_id_计数器生成新的页面ID，计算新页面
     *      需要的文件大小，如果当前文件大小不足，则扩展文件大小。
     *      返回新分配的页面ID。
     * 
     * @return 新分配的页面ID
     */
    int32_t AllocatePage();

    /**
     * @brief 获取数据库文件大小
     * 
     * Why: 需要知道数据库文件的当前大小，以便进行空间管理和计算页面位置。
     *      对于不了解数据库实现的学生来说，这个方法展示了如何获取文件大小，
     *      以及如何使用文件大小信息进行空间计算。
     * 
     * What: GetFileSize方法返回数据库文件的当前大小，以字节为单位。
     *       这个信息可以用来计算数据库中包含的页面数量。
     * 
     * How: 直接返回file_size_成员变量，该变量在构造函数中初始化，
     *      并在分配新页面时更新。
     * 
     * @return 数据库文件大小（以字节为单位）
     */
    size_t GetFileSize() const;

private:
    // 数据库文件路径
    // Why: 需要存储数据库文件的路径，以便在需要时重新打开文件。
    // What: 字符串类型，存储数据库文件的完整路径。
    // How: 在构造函数中初始化，后续不再修改。
    std::string db_file_;

    // 数据库文件流
    // Why: 需要一个文件流对象来进行实际的文件I/O操作。
    // What: fstream对象，用于读写数据库文件。
    // How: 在构造函数中打开，在析构函数中关闭，用于所有的读写操作。
    std::fstream db_io_;

    // 数据库文件大小
    // Why: 需要知道文件大小以计算页面位置和检查边界。
    // What: size_t类型，存储数据库文件的当前大小（字节）。
    // How: 在构造函数中初始化，在分配新页面时更新。
    size_t file_size_;

    // 页面计数器
    // Why: 需要一个计数器来生成唯一的页面ID。
    // What: int32_t类型，存储下一个可用的页面ID。
    // How: 每次分配新页面时递增，确保每个页面ID唯一。
    int32_t next_page_id_;
};

}  // namespace sqlcc