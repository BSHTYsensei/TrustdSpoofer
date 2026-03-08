cat > Tweak.x << 'EOF'
#import <substrate.h>
#import <Foundation/Foundation.h>

// iOS 18 签名验证函数声明
typedef int (*MISValidationFunc)(id, SEL, id, id, id);

// 原始函数指针
MISValidationFunc orig_MISValidateSignature;

// Hook 函数
int hooked_MISValidateSignature(id self, SEL cmd, id arg1, id arg2, id arg3) {
    NSLog(@"[AppSync18] 拦截签名验证，返回成功");
    return 1; // 0 = 失败, 1 = 成功
}

// 初始化函数
__attribute__((constructor))
static void init() {
    @autoreleasepool {
        NSLog(@"[AppSync18] 加载中...");
        
        // 尝试多种方式查找函数
        void *addr = NULL;
        
        // 方式1: 直接符号查找
        addr = dlsym(RTLD_DEFAULT, "MISValidateSignature");
        
        // 方式2: 通过 dyld 遍历
        if (!addr) {
            uint32_t count = _dyld_image_count();
            for (uint32_t i = 0; i < count; i++) {
                const char *name = _dyld_get_image_name(i);
                if (strstr(name, "installd")) {
                    addr = dlsym((void *)RTLD_DEFAULT, "MISValidateSignature");
                    break;
                }
            }
        }
        
        if (addr) {
            MSHookFunction(addr, (void *)hooked_MISValidateSignature, 
                          (void **)&orig_MISValidateSignature);
            NSLog(@"[AppSync18] ✅ Hook 成功 at %p", addr);
        } else {
            NSLog(@"[AppSync18] ❌ 找不到目标函数");
        }
    }
}
EOF
