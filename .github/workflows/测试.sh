cat > scripts/test.sh << 'EOF'
#!/bin/bash
# 本地测试脚本

echo "测试 AppSync iOS18 构建..."

# 设置 Theos
export THEOS=${THEOS:-~/theos}
export PATH=$THEOS/bin:$PATH

# 清理
make clean

# 编译
make package

# 检查结果
if [ -f packages/*.deb ]; then
    echo "✅ 测试通过"
    ls -la packages/
else
    echo "❌ 测试失败"
    exit 1
fi
EOF

chmod +x scripts/test.sh
