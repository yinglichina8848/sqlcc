/**
 * @file config_manager.cc
 * @brief 配置管理器实现文件
 * 
 * Why: 需要实现配置管理器的功能，包括加载、解析、管理和提供配置参数访问接口
 * What: 实现ConfigManager类的所有成员函数，提供配置文件的加载、解析、保存和访问功能
 * How: 使用标准库文件操作、字符串处理和数据结构来实现配置管理功能
 */

#include "config_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <filesystem>

namespace sqlcc {

/**
 * @brief 获取配置管理器单例实例
 * 
 * Why: 需要确保整个应用程序中只有一个配置管理器实例，避免配置冲突
 * What: GetInstance方法返回ConfigManager的单例实例引用
 * How: 使用静态局部变量实现线程安全的单例模式
 */
ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

/**
 * @brief 加载配置文件
 * @param config_file_path 配置文件路径
 * @param env 环境标识，用于加载环境特定配置
 * @return bool 是否加载成功
 * 
 * Why: 需要从文件中加载配置参数，支持不同环境的配置覆盖
 * What: LoadConfig方法加载主配置文件和环境特定配置文件，合并配置参数
 * How: 使用互斥锁保护配置加载过程，先加载默认配置，再加载主配置文件，最后加载环境特定配置
 */
bool ConfigManager::LoadConfig(const std::string& config_file_path, const std::string& env) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 保存配置文件路径和环境标识
    // Why: 需要记录当前配置文件的路径和环境，以便后续重载配置
    // What: 将传入的配置文件路径和环境标识保存到成员变量中
    // How: 直接赋值给成员变量
    config_file_path_ = config_file_path;
    env_ = env;
    
    // 清空当前配置
    // Why: 需要清除之前的配置，避免旧配置与新配置混合
    // What: 清空config_map_中的所有配置项
    // How: 调用clear()方法清空unordered_map
    config_map_.clear();
    
    // 加载默认配置
    // Why: 需要提供默认配置值，确保系统在缺少配置文件时也能正常运行
    // What: 调用LoadDefaultConfig方法加载默认配置参数
    // How: 直接调用LoadDefaultConfig方法
    LoadDefaultConfig();
    
    // 加载主配置文件
    // Why: 需要从主配置文件中加载用户自定义的配置参数
    // What: 调用ParseConfigFile方法解析主配置文件
    // How: 传入配置文件路径，检查解析结果
    if (!ParseConfigFile(config_file_path)) {
        std::cerr << "Failed to load main config file: " << config_file_path << std::endl;
        return false;
    }
    
    // 如果指定了环境，加载环境特定配置文件
    // Why: 需要支持不同环境的配置覆盖，如开发、测试、生产环境
    // What: 根据环境标识构建环境特定配置文件路径，如果文件存在则加载
    // How: 使用filesystem库构建环境配置文件路径，检查文件存在性后加载
    if (!env.empty()) {
        std::filesystem::path config_path(config_file_path);
        std::string env_config_file = config_path.parent_path().string() + "/" + 
                                    config_path.stem().string() + "." + env + 
                                    config_path.extension().string();
        
        // 如果环境配置文件存在，则加载
        // Why: 环境特定配置是可选的，只有文件存在时才加载
        // What: 检查环境配置文件是否存在，存在则调用ParseConfigFile加载
        // How: 使用filesystem::exists检查文件存在性
        if (std::filesystem::exists(env_config_file)) {
            if (!ParseConfigFile(env_config_file)) {
                std::cerr << "Failed to load environment config file: " << env_config_file << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

/**
 * @brief 重新加载配置文件
 * @return bool 是否重新加载成功
 * 
 * Why: 需要在运行时重新加载配置，无需重启应用程序
 * What: ReloadConfig方法使用之前保存的配置文件路径和环境标识重新加载配置
 * How: 检查配置文件路径是否有效，然后调用LoadConfig方法重新加载
 */
bool ConfigManager::ReloadConfig() {
    // 检查配置文件路径是否为空
    // Why: 如果没有保存过配置文件路径，则无法重新加载
    // What: 检查config_file_path_是否为空字符串
    // How: 使用empty()方法检查字符串是否为空
    if (config_file_path_.empty()) {
        return false;
    }
    return LoadConfig(config_file_path_, env_);
}

/**
 * @brief 获取布尔类型配置值
 * @param key 配置键
 * @param default_value 默认值
 * @return bool 配置值
 * 
 * Why: 需要提供获取布尔类型配置值的接口，支持默认值
 * What: GetBool方法从配置中获取指定键的布尔值，如果不存在则返回默认值
 * How: 使用互斥锁保护配置访问，查找配置键并检查类型，返回对应的值或默认值
 */
bool ConfigManager::GetBool(const std::string& key, bool default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 查找配置键
    // Why: 需要在配置映射表中查找指定的键
    // What: 使用find方法在config_map_中查找键
    // How: 调用unordered_map的find方法，返回迭代器
    auto it = config_map_.find(key);
    if (it != config_map_.end() && std::holds_alternative<bool>(it->second)) {
        // 如果找到键且类型为bool，则返回对应的值
        // Why: 需要确保类型匹配，避免类型错误
        // What: 使用std::get<bool>获取bool类型的值
        // How: 调用std::get模板方法
        return std::get<bool>(it->second);
    }
    // 如果键不存在或类型不匹配，返回默认值
    return default_value;
}

/**
 * @brief 获取整数类型配置值
 * @param key 配置键
 * @param default_value 默认值
 * @return int 配置值
 * 
 * Why: 需要提供获取整数类型配置值的接口，支持默认值和类型转换
 * What: GetInt方法从配置中获取指定键的整数值，支持从double类型转换
 * How: 使用互斥锁保护配置访问，查找配置键并检查类型，返回对应的值或默认值
 */
int ConfigManager::GetInt(const std::string& key, int default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 查找配置键
    // Why: 需要在配置映射表中查找指定的键
    // What: 使用find方法在config_map_中查找键
    // How: 调用unordered_map的find方法，返回迭代器
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        // 如果类型为int，直接返回
        // Why: 需要处理int类型的值
        // What: 检查类型并使用std::get<int>获取值
        // How: 使用std::holds_alternative检查类型，std::get获取值
        if (std::holds_alternative<int>(it->second)) {
            return std::get<int>(it->second);
        } else if (std::holds_alternative<double>(it->second)) {
            // 如果类型为double，转换为int返回
            // Why: 需要支持从double到int的类型转换
            // What: 使用static_cast将double转换为int
            // How: 调用static_cast<int>进行类型转换
            return static_cast<int>(std::get<double>(it->second));
        }
    }
    // 如果键不存在或类型不匹配，返回默认值
    return default_value;
}

/**
 * @brief 获取双精度浮点类型配置值
 * @param key 配置键
 * @param default_value 默认值
 * @return double 配置值
 * 
 * Why: 需要提供获取双精度浮点类型配置值的接口，支持默认值和类型转换
 * What: GetDouble方法从配置中获取指定键的double值，支持从int类型转换
 * How: 使用互斥锁保护配置访问，查找配置键并检查类型，返回对应的值或默认值
 */
double ConfigManager::GetDouble(const std::string& key, double default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 查找配置键
    // Why: 需要在配置映射表中查找指定的键
    // What: 使用find方法在config_map_中查找键
    // How: 调用unordered_map的find方法，返回迭代器
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        // 如果类型为double，直接返回
        // Why: 需要处理double类型的值
        // What: 检查类型并使用std::get<double>获取值
        // How: 使用std::holds_alternative检查类型，std::get获取值
        if (std::holds_alternative<double>(it->second)) {
            return std::get<double>(it->second);
        } else if (std::holds_alternative<int>(it->second)) {
            // 如果类型为int，转换为double返回
            // Why: 需要支持从int到double的类型转换
            // What: 使用static_cast将int转换为double
            // How: 调用static_cast<double>进行类型转换
            return static_cast<double>(std::get<int>(it->second));
        }
    }
    // 如果键不存在或类型不匹配，返回默认值
    return default_value;
}

/**
 * @brief 获取字符串类型配置值
 * @param key 配置键
 * @param default_value 默认值
 * @return std::string 配置值
 * 
 * Why: 需要提供获取字符串类型配置值的接口，支持默认值
 * What: GetString方法从配置中获取指定键的字符串值，如果不存在则返回默认值
 * How: 使用互斥锁保护配置访问，查找配置键并检查类型，返回对应的值或默认值
 */
std::string ConfigManager::GetString(const std::string& key, const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 查找配置键
    // Why: 需要在配置映射表中查找指定的键
    // What: 使用find方法在config_map_中查找键
    // How: 调用unordered_map的find方法，返回迭代器
    auto it = config_map_.find(key);
    if (it != config_map_.end() && std::holds_alternative<std::string>(it->second)) {
        // 如果找到键且类型为string，则返回对应的值
        // Why: 需要确保类型匹配，避免类型错误
        // What: 使用std::get<std::string>获取string类型的值
        // How: 调用std::get模板方法
        return std::get<std::string>(it->second);
    }
    // 如果键不存在或类型不匹配，返回默认值
    return default_value;
}

/**
 * @brief 设置配置值
 * @param key 配置键
 * @param value 配置值
 * @return bool 是否设置成功
 * 
 * Why: 需要提供动态设置配置值的接口，支持运行时修改配置
 * What: SetValue方法设置指定键的配置值，如果值发生变化则通知注册的回调函数
 * How: 使用互斥锁保护配置修改，保存旧值，设置新值，比较新旧值并通知回调
 */
bool ConfigManager::SetValue(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 保存旧值，用于比较
    // Why: 需要比较新旧值，只有值发生变化时才通知回调
    // What: 保存当前键对应的值，用于后续比较
    // How: 查找键并保存对应的值
    ConfigValue old_value;
    bool has_old_value = false;
    
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        old_value = it->second;
        has_old_value = true;
    }
    
    // 设置新值
    // Why: 需要将新值保存到配置映射表中
    // What: 将传入的值赋值给指定键
    // How: 使用operator[]赋值
    config_map_[key] = value;
    
    // 如果值发生变化，通知回调
    // Why: 需要通知注册的回调函数配置值已更改
    // What: 比较新旧值，如果不同则调用NotifyConfigChange
    // How: 检查has_old_value和比较old_value与value
    if (!has_old_value || old_value != value) {
        NotifyConfigChange(key, value);
    }
    
    return true;
}

/**
 * @brief 检查配置键是否存在
 * @param key 配置键
 * @return bool 是否存在
 * 
 * Why: 需要提供检查配置键是否存在的接口
 * What: HasKey方法检查指定键是否存在于配置映射表中
 * How: 使用互斥锁保护配置访问，使用find方法查找键
 */
bool ConfigManager::HasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    // 查找键并检查是否找到
    // Why: 需要确定键是否存在于配置中
    // What: 使用find方法查找键，检查迭代器是否等于end()
    // How: 调用unordered_map的find方法，比较返回的迭代器
    return config_map_.find(key) != config_map_.end();
}

/**
 * @brief 注册配置变更回调函数
 * @param key 配置键
 * @param callback 回调函数
 * @return int 回调ID，用于取消注册
 * 
 * Why: 需要提供监听配置变更的机制，使其他模块能够响应配置变化
 * What: RegisterChangeCallback方法为指定键注册回调函数，返回回调ID
 * How: 使用互斥锁保护回调注册，生成唯一ID，保存回调函数
 */
int ConfigManager::RegisterChangeCallback(const std::string& key, ConfigChangeCallback callback) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 生成唯一回调ID
    // Why: 需要为每个回调函数分配唯一ID，以便后续取消注册
    // What: 使用next_callback_id_生成唯一ID并递增
    // How: 将next_callback_id_作为ID，然后递增
    int callback_id = next_callback_id_++;
    
    // 保存回调函数
    // Why: 需要将回调函数与键关联，以便配置变更时调用
    // What: 将回调ID和回调函数保存到callbacks_映射表中
    // How: 使用emplace_back添加到对应键的回调列表中
    callbacks_[key].emplace_back(callback_id, callback);
    
    return callback_id;
}

/**
 * @brief 取消注册配置变更回调函数
 * @param callback_id 回调ID
 * @return bool 是否取消成功
 * 
 * Why: 需要提供取消注册回调函数的机制，避免不再需要的回调被调用
 * What: UnregisterChangeCallback方法根据回调ID查找并移除对应的回调函数
 * How: 使用互斥锁保护回调取消，遍历所有键的回调列表，查找并移除匹配的回调ID
 */
bool ConfigManager::UnregisterChangeCallback(int callback_id) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 遍历所有键的回调列表
    // Why: 需要在所有键的回调列表中查找指定的回调ID
    // What: 使用范围for循环遍历callbacks_映射表
    // How: 使用结构化绑定获取键和回调列表
    for (auto& [key, callback_list] : callbacks_) {
        // 查找匹配的回调ID
        // Why: 需要在回调列表中找到指定ID的回调函数
        // What: 使用find_if算法查找匹配的回调ID
        // How: 使用lambda表达式比较回调ID
        auto it = std::find_if(callback_list.begin(), callback_list.end(),
            [callback_id](const std::pair<int, ConfigChangeCallback>& pair) {
                return pair.first == callback_id;
            });
        
        // 如果找到，移除并返回成功
        // Why: 需要移除找到的回调函数并返回成功状态
        // What: 使用erase方法移除找到的回调
        // How: 调用vector的erase方法
        if (it != callback_list.end()) {
            callback_list.erase(it);
            return true;
        }
    }
    
    // 未找到匹配的回调ID，返回失败
    return false;
}

/**
 * @brief 保存当前配置到文件
 * @param file_path 文件路径
 * @return bool 是否保存成功
 * 
 * Why: 需要提供将当前配置保存到文件的接口，支持配置持久化
 * What: SaveToFile方法将当前配置映射表中的所有配置项写入指定文件
 * How: 使用互斥锁保护配置访问，按节组织配置项，写入文件
 */
bool ConfigManager::SaveToFile(const std::string& file_path) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 打开文件进行写入
    // Why: 需要创建文件并写入配置内容
    // What: 使用ofstream打开文件进行写入
    // How: 创建ofstream对象并检查是否成功打开
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // 写入文件头注释
    // Why: 需要在配置文件开头添加注释，说明文件用途
    // What: 写入SQLCC配置文件的注释
    // How: 使用流插入运算符写入注释行
    file << "# SQLCC 配置文件\n";
    file << "# 由ConfigManager自动生成\n\n";
    
    // 跟踪当前节名，用于生成节标题
    // Why: 需要跟踪当前节，以便在节变化时写入节标题
    // What: 使用current_section变量保存当前处理的节名
    // How: 使用字符串变量保存节名
    std::string current_section;
    
    // 遍历所有配置项
    // Why: 需要将所有配置项写入文件
    // What: 使用范围for循环遍历config_map_
    // How: 使用结构化绑定获取键和值
    for (const auto& [key, value] : config_map_) {
        // 提取节名
        // Why: 需要从键中提取节名，用于组织配置文件结构
        // What: 查找键中的第一个点号，分割节名和参数名
        // How: 使用find方法查找点号，使用substr提取节名
        size_t dot_pos = key.find('.');
        if (dot_pos != std::string::npos) {
            std::string section = key.substr(0, dot_pos);
            // 如果节名发生变化，写入节标题
            // Why: 需要在节变化时写入节标题，组织配置文件结构
            // What: 比较当前节名和之前节名，不同则写入节标题
            // How: 使用if语句比较节名，写入节标题
            if (section != current_section) {
                current_section = section;
                file << "[" << section << "]\n";
            }
            
            // 提取参数名
            // Why: 需要从键中提取参数名，用于写入配置项
            // What: 获取点号后的部分作为参数名
            // How: 使用substr提取点号后的部分
            std::string param = key.substr(dot_pos + 1);
            file << param << " = ";
            
            // 根据值类型写入对应的字符串表示
            // Why: 需要根据值的类型转换为对应的字符串表示
            // What: 使用std::holds_alternative检查类型，使用std::get获取值
            // How: 使用if-else语句处理不同类型
            if (std::holds_alternative<bool>(value)) {
                file << (std::get<bool>(value) ? "true" : "false");
            } else if (std::holds_alternative<int>(value)) {
                file << std::get<int>(value);
            } else if (std::holds_alternative<double>(value)) {
                file << std::get<double>(value);
            } else if (std::holds_alternative<std::string>(value)) {
                file << std::get<std::string>(value);
            }
            
            // 写入换行符
            file << "\n";
        }
    }
    
    return true;
}

/**
 * @brief 获取所有配置键
 * @return std::vector<std::string> 配置键列表
 * 
 * Why: 需要提供获取所有配置键的接口，用于配置浏览和调试
 * What: GetAllKeys方法返回配置映射表中所有键的列表
 * How: 使用互斥锁保护配置访问，遍历配置映射表，收集所有键
 */
std::vector<std::string> ConfigManager::GetAllKeys() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 创建键列表，预留空间
    // Why: 需要创建一个vector来存储所有键，预留空间可以提高性能
    // What: 创建vector并预留config_map_.size()的空间
    // How: 使用reserve方法预留空间
    std::vector<std::string> keys;
    keys.reserve(config_map_.size());
    
    // 遍历配置映射表，收集所有键
    // Why: 需要将所有键添加到vector中
    // What: 使用范围for循环遍历config_map_，将键添加到vector
    // How: 使用结构化绑定获取键，使用push_back添加
    for (const auto& [key, _] : config_map_) {
        keys.push_back(key);
    }
    
    return keys;
}

/**
 * @brief 获取指定前缀的所有配置键
 * @param prefix 键前缀
 * @return std::vector<std::string> 配置键列表
 * 
 * Why: 需要提供按前缀筛选配置键的接口，用于分类获取配置
 * What: GetKeysWithPrefix方法返回所有以指定前缀开头的配置键列表
 * How: 使用互斥锁保护配置访问，遍历配置映射表，筛选匹配前缀的键
 */
std::vector<std::string> ConfigManager::GetKeysWithPrefix(const std::string& prefix) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 创建键列表
    // Why: 需要创建一个vector来存储匹配前缀的键
    // What: 创建vector用于存储结果
    // How: 直接创建空的vector
    std::vector<std::string> keys;
    
    // 遍历配置映射表，筛选匹配前缀的键
    // Why: 需要找到所有以指定前缀开头的键
    // What: 使用范围for循环遍历config_map_，检查键的前缀
    // How: 使用substr方法获取键的前缀部分，进行比较
    for (const auto& [key, _] : config_map_) {
        if (key.substr(0, prefix.length()) == prefix) {
            keys.push_back(key);
        }
    }
    
    return keys;
}

/**
 * @brief 解析配置文件
 * @param file_path 文件路径
 * @return bool 是否解析成功
 * 
 * Why: 需要从文件中读取并解析配置内容
 * What: ParseConfigFile方法逐行读取配置文件，解析配置项
 * How: 使用ifstream打开文件，逐行读取并调用ParseConfigLine解析
 */
bool ConfigManager::ParseConfigFile(const std::string& file_path) {
    // 打开文件进行读取
    // Why: 需要从文件中读取配置内容
    // What: 使用ifstream打开文件进行读取
    // How: 创建ifstream对象并检查是否成功打开
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    // 逐行读取文件
    // Why: 需要逐行处理配置文件内容
    // What: 使用getline逐行读取文件内容
    // How: 使用while循环和getline读取每一行
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        // 解析每一行
        // Why: 需要解析每一行的配置内容
        // What: 调用ParseConfigLine方法解析行内容
        // How: 传入行内容和当前节引用
        ParseConfigLine(line, current_section);
    }
    
    return true;
}

/**
 * @brief 解析配置行
 * @param line 配置行
 * @param current_section 当前配置节
 * @return bool 是否解析成功
 * 
 * Why: 需要解析单行配置内容，支持节、键值对和注释
 * What: ParseConfigLine方法解析单行配置，处理节行、键值对行和注释行
 * How: 去除空白字符，根据行首字符判断行类型，进行相应处理
 */
bool ConfigManager::ParseConfigLine(const std::string& line, std::string& current_section) {
    // 去除首尾空白字符
    // Why: 需要去除行首尾的空白字符，便于后续解析
    // What: 使用erase和find_first/last_not_of去除空白字符
    // How: 先去除行首空白，再去除行尾空白
    std::string trimmed_line = line;
    trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t"));
    trimmed_line.erase(trimmed_line.find_last_not_of(" \t") + 1);
    
    // 跳过空行和注释行
    // Why: 空行和注释行不包含配置信息，需要跳过
    // What: 检查行是否为空或以#开头
    // How: 使用empty和下标访问检查行内容
    if (trimmed_line.empty() || trimmed_line[0] == '#') {
        return true;
    }
    
    // 处理节行 [section]
    // Why: 需要识别并处理节行，更新当前节名
    // What: 检查行是否以[开头和]结尾，提取节名
    // How: 使用下标访问检查首尾字符，使用substr提取节名
    if (trimmed_line[0] == '[' && trimmed_line.back() == ']') {
        current_section = trimmed_line.substr(1, trimmed_line.length() - 2);
        return true;
    }
    
    // 处理键值对行 key = value
    // Why: 需要识别并处理键值对行，提取键和值
    // What: 查找等号位置，分割键和值，去除空白字符
    // How: 使用find查找等号，使用substr分割，使用erase去除空白
    size_t equal_pos = trimmed_line.find('=');
    if (equal_pos != std::string::npos) {
        std::string key = trimmed_line.substr(0, equal_pos);
        std::string value = trimmed_line.substr(equal_pos + 1);
        
        // 去除键和值的空白字符
        // Why: 键和值周围可能有空白字符，需要去除
        // What: 使用erase和find_first/last_not_of去除空白字符
        // How: 与去除行空白类似的方式处理键和值
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // 构建完整键名 section.key
        // Why: 需要将节名和参数名组合成完整键名
        // What: 如果有当前节名，则组合成"section.key"格式
        // How: 使用字符串拼接和条件判断
        std::string full_key = current_section.empty() ? key : current_section + "." + key;
        
        // 解析值类型并存储
        // Why: 需要根据值的字符串表示解析为合适的类型
        // What: 检查值是否为布尔值、浮点数或整数，进行相应转换
        // How: 使用字符串比较和try-catch进行类型转换
        if (value == "true" || value == "false") {
            config_map_[full_key] = (value == "true");
        } else if (value.find('.') != std::string::npos) {
            try {
                config_map_[full_key] = std::stod(value);
            } catch (const std::exception&) {
                config_map_[full_key] = value;
            }
        } else {
            try {
                config_map_[full_key] = std::stoi(value);
            } catch (const std::exception&) {
                config_map_[full_key] = value;
            }
        }
        
        return true;
    }
    
    return false;
}

/**
 * @brief 加载默认配置
 * 
 * Why: 需要提供默认配置值，确保系统在缺少配置文件时也能正常运行
 * What: LoadDefaultConfig方法设置所有配置项的默认值
 * How: 直接在config_map_中设置各个配置项的默认值
 */
void ConfigManager::LoadDefaultConfig() {
    // 数据库配置
    // Why: 需要设置数据库相关的默认配置
    // What: 设置数据库文件路径、大小限制和页面大小等配置
    // How: 直接在config_map_中设置键值对
    config_map_["database.db_file_path"] = std::string("./data/sqlcc.db");
    config_map_["database.db_file_size_limit"] = 1024;
    config_map_["database.page_size"] = 8192;
    
    // 缓冲池配置
    // Why: 需要设置缓冲池相关的默认配置
    // What: 设置缓冲池大小、替换策略、预取策略等配置
    // How: 直接在config_map_中设置键值对
    config_map_["buffer_pool.pool_size"] = 64;
    config_map_["buffer_pool.replacement_policy"] = std::string("LRU");
    config_map_["buffer_pool.prefetch_strategy"] = std::string("SEQUENTIAL");
    config_map_["buffer_pool.prefetch_window"] = 4;
    config_map_["buffer_pool.flush_interval"] = 30;
    config_map_["buffer_pool.dirty_page_threshold"] = 0.75;
    
    // 磁盘管理器配置
    // Why: 需要设置磁盘管理器相关的默认配置
    // What: 设置IO线程池大小、批量读写大小、异步IO等配置
    // How: 直接在config_map_中设置键值对
    config_map_["disk_manager.io_thread_pool_size"] = 4;
    config_map_["disk_manager.batch_read_size"] = 8;
    config_map_["disk_manager.batch_write_size"] = 8;
    config_map_["disk_manager.async_io"] = true;
    config_map_["disk_manager.direct_io"] = false;
    config_map_["disk_manager.io_scheduler"] = std::string("FIFO");
    
    // 存储引擎配置
    // Why: 需要设置存储引擎相关的默认配置
    // What: 设置并发控制、锁超时、隔离级别等配置
    // How: 直接在config_map_中设置键值对
    config_map_["storage_engine.concurrency_control"] = std::string("PESSIMISTIC");
    config_map_["storage_engine.lock_timeout"] = 5000;
    config_map_["storage_engine.deadlock_detection_interval"] = 1000;
    config_map_["storage_engine.isolation_level"] = std::string("READ_COMMITTED");
    config_map_["storage_engine.checkpoint_interval"] = 60;
    
    // 日志配置
    // Why: 需要设置日志相关的默认配置
    // What: 设置日志级别、日志文件路径、日志文件大小限制等配置
    // How: 直接在config_map_中设置键值对
    config_map_["logging.log_level"] = std::string("INFO");
    config_map_["logging.log_file_path"] = std::string("./logs/sqlcc.log");
    config_map_["logging.log_file_size_limit"] = 100;
    config_map_["logging.log_file_backup_count"] = 5;
    config_map_["logging.log_to_console"] = true;
    
    // 性能配置
    // Why: 需要设置性能相关的默认配置
    // What: 设置性能监控、统计间隔、性能分析等配置
    // How: 直接在config_map_中设置键值对
    config_map_["performance.enable_monitoring"] = false;
    config_map_["performance.stats_interval"] = 10;
    config_map_["performance.stats_output_path"] = std::string("./stats/");
    config_map_["performance.enable_profiling"] = false;
    
    // 测试配置
    // Why: 需要设置测试相关的默认配置
    // What: 设置测试模式、测试数据目录、测试输出目录等配置
    // How: 直接在config_map_中设置键值对
    config_map_["testing.test_mode"] = false;
    config_map_["testing.test_data_dir"] = std::string("./test_data/");
    config_map_["testing.test_output_dir"] = std::string("./test_results/");
    config_map_["testing.verbose_test_log"] = false;
}

/**
 * @brief 通知配置变更
 * @param key 配置键
 * @param new_value 新值
 * 
 * Why: 需要通知注册的回调函数配置值已更改
 * What: NotifyConfigChange方法查找指定键的所有回调函数并调用它们
 * How: 在callbacks_中查找键，遍历所有回调函数并调用，捕获异常
 */
void ConfigManager::NotifyConfigChange(const std::string& key, const ConfigValue& new_value) {
    // 查找键对应的回调函数列表
    // Why: 需要找到注册了该键变更通知的所有回调函数
    // What: 在callbacks_映射表中查找键
    // How: 使用find方法查找键
    auto it = callbacks_.find(key);
    if (it != callbacks_.end()) {
        // 遍历所有回调函数并调用
        // Why: 需要通知所有注册的回调函数配置已更改
        // What: 遍历回调列表，调用每个回调函数
        // How: 使用范围for循环遍历回调列表
        for (const auto& [id, callback] : it->second) {
            try {
                // 调用回调函数
                // Why: 需要将键和新值传递给回调函数
                // What: 调用回调函数并传递参数
                // How: 使用函数调用语法
                callback(key, new_value);
            } catch (const std::exception& e) {
                // 捕获并处理回调函数中的异常
                // Why: 需要防止回调函数中的异常影响其他回调函数的执行
                // What: 捕获异常并输出错误信息
                // How: 使用try-catch捕获异常，使用cerr输出错误信息
                std::cerr << "Error in config change callback for key " << key 
                         << ": " << e.what() << std::endl;
            }
        }
    }
}

}  // namespace sqlcc