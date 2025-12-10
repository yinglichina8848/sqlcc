# Gitee SSH 密钥配置指南

## 1. 生成 SSH 密钥对

如果您还没有SSH密钥对，可以使用以下命令生成：

```bash
ssh-keygen -t rsa -b 4096 -C "your_email@example.com" -f ~/.ssh/id_rsa_sqlcc -N ""
```

参数说明：
- `-t rsa`: 指定密钥类型为RSA
- `-b 4096`: 指定密钥长度为4096位
- `-C "your_email@example.com"`: 添加注释（通常是您的邮箱）
- `-f ~/.ssh/id_rsa_sqlcc`: 指定密钥文件路径和名称
- `-N ""`: 不设置密码短语

## 2. 查看并复制公钥

生成密钥后，查看公钥内容：

```bash
cat ~/.ssh/id_rsa_sqlcc.pub
```

复制显示的全部内容，这将是您需要添加到Gitee的SSH公钥。

## 3. 添加 SSH 公钥到 Gitee

### 步骤 1: 登录 Gitee 账户
访问 [Gitee](https://gitee.com) 并登录您的账户。

### 步骤 2: 进入 SSH 公钥管理页面
1. 点击右上角的头像
2. 选择「设置」
3. 在左侧菜单中选择「SSH公钥」

### 步骤 3: 添加公钥
1. 在「标题」栏填写一个易于识别的名称，例如：`SQLCC-Development-Key`
2. 在「公钥」文本框中粘贴您之前复制的公钥内容
3. 点击「确定」按钮完成添加

## 4. 配置 SSH 客户端

创建或编辑 SSH 配置文件：

```bash
nano ~/.ssh/config
```

添加以下内容：

```
Host gitee.com
  HostName gitee.com
  PreferredAuthentications publickey
  IdentityFile ~/.ssh/id_rsa_sqlcc
```

## 5. 测试 SSH 连接

测试与 Gitee 的连接：

```bash
ssh -T git@gitee.com
```

如果看到类似以下的消息，表示配置成功：
```
Hi your_username! You've successfully authenticated, but GITEE.COM does not provide shell access.
```

## 6. 修改远程仓库 URL（如果需要）

如果您的仓库目前使用 HTTPS 方式，需要更改为 SSH 方式：

```bash
cd /home/liying/sqlcc
git remote set-url origin git@gitee.com:yinglichina/sqlcc.git
```

## 7. 验证和推送

验证远程仓库 URL：

```bash
git remote -v
```

应该显示类似：
```
origin  git@gitee.com:yinglichina/sqlcc.git (fetch)
origin  git@gitee.com:yinglichina/sqlcc.git (push)
```

现在您可以正常推送代码了：

```bash
git push origin main
```

## 常见问题解决

### 1. 权限问题
确保 SSH 密钥文件权限正确：

```bash
chmod 700 ~/.ssh
chmod 600 ~/.ssh/id_rsa_sqlcc
chmod 644 ~/.ssh/id_rsa_sqlcc.pub
```

### 2. 连接被拒绝
如果遇到连接问题，可以尝试使用不同的 SSH 端口：

```bash
ssh -T -p 443 git@gitee.com
```

### 3. 多个 SSH 密钥管理
如果您有多个 SSH 密钥，确保在 `~/.ssh/config` 文件中正确配置每个主机的密钥。

## 注意事项

1. **私钥安全**：私钥文件（id_rsa_sqlcc）必须保密，不要分享给任何人
2. **公钥共享**：公钥文件（id_rsa_sqlcc.pub）可以安全地添加到Gitee等代码托管平台
3. **备份密钥**：建议备份您的SSH密钥对，以防丢失
4. **定期更换**：出于安全考虑，建议定期更换SSH密钥

通过以上步骤，您就可以成功配置SSH密钥并推送到Gitee了。