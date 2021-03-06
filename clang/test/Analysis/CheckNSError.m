// RUN: %clang_cc1 -analyze -analyzer-experimental-internal-checks -analyzer-check-objc-mem -analyzer-store=basic -analyzer-constraints=basic -verify %s
// RUN: %clang_cc1 -analyze -analyzer-experimental-internal-checks -analyzer-check-objc-mem -analyzer-store=region -analyzer-constraints=basic -verify %s
// RUN: %clang_cc1 -analyze -analyzer-experimental-internal-checks -analyzer-check-objc-mem -analyzer-store=basic -analyzer-constraints=range -verify %s
// RUN: %clang_cc1 -analyze -analyzer-experimental-internal-checks -analyzer-check-objc-mem -analyzer-store=region -analyzer-constraints=range -verify %s


typedef signed char BOOL;
typedef int NSInteger;
typedef struct _NSZone NSZone;
@class NSInvocation, NSMethodSignature, NSCoder, NSString, NSEnumerator;
@protocol NSObject  - (BOOL)isEqual:(id)object; @end
@protocol NSCopying  - (id)copyWithZone:(NSZone *)zone; @end
@protocol NSCoding  - (void)encodeWithCoder:(NSCoder *)aCoder; @end
@interface NSObject <NSObject> {} @end
@class NSDictionary;
@interface NSError : NSObject <NSCopying, NSCoding> {}
+ (id)errorWithDomain:(NSString *)domain code:(NSInteger)code userInfo:(NSDictionary *)dict;
@end
extern NSString * const NSXMLParserErrorDomain ;

@interface A
- (void)myMethodWhichMayFail:(NSError **)error;
- (BOOL)myMethodWhichMayFail2:(NSError **)error;
@end

@implementation A
- (void)myMethodWhichMayFail:(NSError **)error {   // expected-warning {{Method accepting NSError** should have a non-void return value to indicate whether or not an error occurred}}
  *error = [NSError errorWithDomain:@"domain" code:1 userInfo:0]; // expected-warning {{Potential null dereference.}}
}

- (BOOL)myMethodWhichMayFail2:(NSError **)error {  // no-warning
  if (error) *error = [NSError errorWithDomain:@"domain" code:1 userInfo:0]; // no-warning
  return 0;
}
@end

struct __CFError {};
typedef struct __CFError* CFErrorRef;

void foo(CFErrorRef* error) { // expected-warning {{Function accepting CFErrorRef* should have a non-void return value to indicate whether or not an error occurred}}
  *error = 0;  // expected-warning {{Potential null dereference.}}
}

int f1(CFErrorRef* error) {
  if (error) *error = 0; // no-warning
  return 0;
}

int f2(CFErrorRef* error) {
  if (0 != error) *error = 0; // no-warning
  return 0;
}

int f3(CFErrorRef* error) {
  if (error != 0) *error = 0; // no-warning
  return 0;
}


