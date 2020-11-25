//
//  osxInAppPurchases.m
//  xLights
//
//  Created by Daniel Kulp on 11/24/20.
//  Copyright © 2020 Daniel Kulp. All rights reserved.

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

typedef void (^RequestProductsCompletionHandler)(BOOL success, NSArray * products);

@interface IAPHelper : NSObject
- (id)initWithProductIdentifiers:(NSSet *)productIdentifiers;
- (void)requestProductsWithCompletionHandler:(RequestProductsCompletionHandler)completionHandler;


- (void)buyProduct:(SKProduct *)product;
- (BOOL)productPurchased:(NSString *)productIdentifier;

- (void)restoreCompletedTransactions;
@end

@interface IAPHelper () <SKProductsRequestDelegate, SKPaymentTransactionObserver>
@end
@implementation IAPHelper
{
    // You create an instance variable to store the SKProductsRequest you will issue to retrieve a list of products, while it is active.
    SKProductsRequest *_productsRequest;
    // You also keep track of the completion handler for the outstanding products request, ...
    RequestProductsCompletionHandler _completionHandler;
    // ... the list of product identifiers passed in, ...
    NSSet *_productIdentifiers;
    // ... and the list of product identifiers that have been previously purchased.
    NSMutableSet * _purchasedProductIdentifiers;
}

- (id)initWithProductIdentifiers:(NSSet *)productIdentifiers {
    self = [super init];
    if (self) {
        // Store product identifiers
        _productIdentifiers = productIdentifiers;
        // Check for previously purchased products
        _purchasedProductIdentifiers = [NSMutableSet set];
        /*
        for (NSString * productIdentifier in _productIdentifiers) {
            BOOL productPurchased = [[NSUserDefaults standardUserDefaults] boolForKey:productIdentifier];
            if (productPurchased) {
                [_purchasedProductIdentifiers addObject:productIdentifier];
                NSLog(@"Previously purchased: %@", productIdentifier);
            } else {
                NSLog(@"Not purchased: %@", productIdentifier);
            }
        }
        */
    }
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
    return self;
}
- (void)requestProductsWithCompletionHandler:(RequestProductsCompletionHandler)completionHandler
{
    // a copy of the completion handler block inside the instance variable
    _completionHandler = [completionHandler copy];
    // Create a new instance of SKProductsRequest, which is the Apple-written class that contains the code to pull the info from iTunes Connect
    _productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:_productIdentifiers];
    _productsRequest.delegate = self;
    [_productsRequest start];
}

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    NSLog(@"Loaded products...");
    _productsRequest = nil;
    NSArray * skProducts = response.products;
    for (SKProduct * skProduct in skProducts) {
        NSLog(@"Found product: %@ – Product: %@ – Price: %0.2f", skProduct.productIdentifier, skProduct.localizedTitle, skProduct.price.floatValue);
    }
    _completionHandler(YES, skProducts);
    _completionHandler = nil;
}
- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    NSLog(@"Failed to load list of products.");
    _productsRequest = nil;
    _completionHandler(NO, nil);
    _completionHandler = nil;
}
- (void)paymentQueue:(nonnull SKPaymentQueue *)queue updatedTransactions:(nonnull NSArray<SKPaymentTransaction *> *)transactions {
    for (SKPaymentTransaction *transaction in transactions) {
        switch (transaction.transactionState) {
            case SKPaymentTransactionStatePurchased:
                [self completeTransaction:transaction];
                break;
            case SKPaymentTransactionStateFailed:
                [self failedTransaction:transaction];
                break;
            case SKPaymentTransactionStateRestored:
                [self restoreTransaction:transaction];
            default:
                break;
        }
    };
}

- (void)completeTransaction:(SKPaymentTransaction *)transaction
{
    NSLog(@"completeTransaction...");
   // [self provideContentForProductIdentifier:transaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
    
    
    dispatch_sync(dispatch_get_main_queue(), ^(){
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Thank you!"];
        [alert setInformativeText:@"Thank you for your support!"];
        [alert addButtonWithTitle:@"Ok"];
        [alert runModal];
    });

    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:transaction.payment.productIdentifier];
    [[NSUserDefaults standardUserDefaults] synchronize];
}
- (void)restoreTransaction:(SKPaymentTransaction *)transaction
{
    NSLog(@"restoreTransaction...");
    //[self provideContentForProductIdentifier:transaction.originalTransaction.payment.productIdentifier];
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:transaction.originalTransaction.payment.productIdentifier];
    [[NSUserDefaults standardUserDefaults] synchronize];
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}
- (void)failedTransaction:(SKPaymentTransaction *)transaction
{
    NSLog(@"failedTransaction...");
    if (transaction.error.code != SKErrorPaymentCancelled) {
        NSLog(@"Transaction error: %@", transaction.error.localizedDescription);
        dispatch_sync(dispatch_get_main_queue(), ^(){
            NSAlert *alert = [[NSAlert alloc] init];
            [alert setMessageText:@"Failed Purchase."];
            [alert setInformativeText:transaction.error.localizedDescription];
            [alert addButtonWithTitle:@"Ok"];
            [alert runModal];
        });
    }
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
}
- (BOOL)productPurchased:(NSString *)productIdentifier
{
    return [_purchasedProductIdentifiers containsObject:productIdentifier];
}
 
- (void)buyProduct:(SKProduct *)product
{
    NSLog(@"Buying %@...", product.productIdentifier);
    SKPayment * payment = [SKPayment paymentWithProduct:product];
    [[SKPaymentQueue defaultQueue] addPayment:payment];
}



- (void)restoreCompletedTransactions {
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}
@end


static NSString *kIdentifierOneTimeSmall = @"org.xlights.xLightsOneTimeSmall";
static NSString *kIdentifierOneTimeMedium = @"org.xlights.xLightsOneTimeMedium";
static NSString *kIdentifierOneTimeLarge = @"org.xlights.xLightsOneTimeLarge";
static NSString *kIdentifierMonthlySmall = @"org.xlights.xLightsMonthlySmall";
static NSString *kIdentifierMonthlyMedium = @"org.xlights.xLightsMonthlyMedium";
static NSString *kIdentifierMonthlyLarge = @"org.xlights.xLightsMonthlyLarge";

@interface xLightsIAPHelper : IAPHelper
+ (xLightsIAPHelper *)sharedInstance;
@end
@implementation xLightsIAPHelper
 
+ (xLightsIAPHelper *)sharedInstance {
    static xLightsIAPHelper *sharedInstance;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSSet *productIdentifiers = [NSSet setWithObjects:
                                     kIdentifierOneTimeSmall,
                                     kIdentifierOneTimeMedium,
                                     kIdentifierOneTimeLarge,
                                     kIdentifierMonthlySmall,
                                     kIdentifierMonthlyMedium,
                                     kIdentifierMonthlyLarge,
                                     nil];
        sharedInstance = [[self alloc] initWithProductIdentifiers:productIdentifiers];
    });
    return sharedInstance;
}
@end

#include <list>
#include <string>
#include <thread>
#include <chrono>
#include "InAppPurchaseDialog.h"
class wxWindow;


bool DoInAppPurchases(wxWindow *w) {
    __block bool done = false;
    __block InAppPurchaseDialog *dlg = new InAppPurchaseDialog(w);
    __block NSArray *returnedProducts = nil;
    [[xLightsIAPHelper sharedInstance] requestProductsWithCompletionHandler:^(BOOL success, NSArray *products) {
            if (success) {
                returnedProducts = [NSMutableArray arrayWithArray:products];
                [returnedProducts retain];
                for (SKProduct* product in products) {
                    // do something with object
                    NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
                    [numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
                    [numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
                    [numberFormatter setLocale:product.priceLocale];
                    NSString *price = [numberFormatter stringFromNumber:product.price];
                    if ([product.productIdentifier isEqualToString:kIdentifierOneTimeSmall]) {
                        dlg->prices[0] = std::string([price UTF8String]);
                    }
                    if ([product.productIdentifier isEqualToString:kIdentifierOneTimeMedium]) {
                        dlg->prices[1] = std::string([price UTF8String]);
                    }
                    if ([product.productIdentifier isEqualToString:kIdentifierOneTimeLarge]) {
                        dlg->prices[2] = std::string([price UTF8String]);
                    }
                    if ([product.productIdentifier isEqualToString:kIdentifierMonthlySmall]) {
                        dlg->prices[3] = std::string([price UTF8String]);
                    }
                    if ([product.productIdentifier isEqualToString:kIdentifierMonthlyMedium]) {
                        dlg->prices[4] = std::string([price UTF8String]);
                    }
                    if ([product.productIdentifier isEqualToString:kIdentifierMonthlyLarge]) {
                        dlg->prices[5] = std::string([price UTF8String]);
                    }
                }
            }
            done = true;
        }];
    
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (returnedProducts == nil) {
        return false;
    }
    dlg->setPrices();
    if (dlg->ShowModal() == wxID_OK) {
        int i = dlg->getSelection();
        if (i) {
            for (SKProduct* product in returnedProducts) {
                if (i == 1 && [product.productIdentifier isEqualToString:kIdentifierOneTimeSmall]) {
                    [[xLightsIAPHelper sharedInstance] buyProduct:product];
                } else if (i == 2 && [product.productIdentifier isEqualToString:kIdentifierOneTimeMedium]) {
                    [[xLightsIAPHelper sharedInstance] buyProduct:product];
                } else if (i == 3 && [product.productIdentifier isEqualToString:kIdentifierOneTimeLarge]) {
                    [[xLightsIAPHelper sharedInstance] buyProduct:product];
                } else if (i == 4 && [product.productIdentifier isEqualToString:kIdentifierMonthlySmall]) {
                    [[xLightsIAPHelper sharedInstance] buyProduct:product];
                } else if (i == 5 && [product.productIdentifier isEqualToString:kIdentifierMonthlyMedium]) {
                    [[xLightsIAPHelper sharedInstance] buyProduct:product];
                } else if (i == 6 && [product.productIdentifier isEqualToString:kIdentifierMonthlyLarge]) {
                    [[xLightsIAPHelper sharedInstance] buyProduct:product];
                }
            }
        }
        [returnedProducts release];
        return true;
    }
    [returnedProducts release];
    return false;
}
