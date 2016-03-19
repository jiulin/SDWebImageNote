/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import "SDWebImageCompat.h"
#import "SDWebImageOperation.h"
#import "SDWebImageDownloader.h"
#import "SDImageCache.h"

typedef NS_OPTIONS(NSUInteger, SDWebImageOptions) {
    /**
     * By default, when a URL fail to be downloaded, the URL is blacklisted so the library won't keep trying.
     * This flag disable this blacklisting.
     */
    /**
     *  默认情况下当一个url下载失败，这个url会被置于黑名单，Sd的库不会继续下载。
     *  这个SDWebImageRetryFailed标记就是不允许这种将下载失败的url加入黑名单的行为。
     *  换句话说，就是选择了该标记后，url下载失败还会继续下载
     *
     *  SDWebImageRetryFailed 意思是第一次使用url1请求失败，默认情况下下次使用url1请求的时候则会直接跳过请求。而设置了SDWebImageRetryFailed标记后，下次使用url1请求的还会再次请求。
     *  这里有个误区就是，有人认为设置SDWebImageRetryFailed，则图片下载失败，sd还会自动再次请求。源码并无此行为，ImageRetryFailed是手动触发的，下次使用相同的url的时候会触发。
    */
    SDWebImageRetryFailed = 1 << 0,

    /**
     * By default, image downloads are started during UI interactions, this flags disable this feature,
     * leading to delayed download on UIScrollView deceleration for instance.
     */
    /**
     *  默认情况下，图片在UI交互过程中开始下载，这个标记禁止这种功能，从而让导致延迟加载。以UIScrollView为例，也就是SCrollView滑动的时候不会下载，只有当滑动减速的时候才开始下载。
     */
    SDWebImageLowPriority = 1 << 1,

    /**
     * This flag disables on-disk caching
     */
    /**
     *  这个标记禁止磁盘缓存，只允许内存缓存
     */
    SDWebImageCacheMemoryOnly = 1 << 2,

    /**
     * This flag enables progressive download, the image is displayed progressively during download as a browser would do.
     * By default, the image is only displayed once completely downloaded.
     */
    /**
     *  这个标记允许逐步下载，图片可以在下载过程中逐渐显示
     *  默认情况下，图片只有在完全下载完成才会显示
     */
    SDWebImageProgressiveDownload = 1 << 3,

    /**
     * Even if the image is cached, respect the HTTP response cache control, and refresh the image from remote location if needed.
     * The disk caching will be handled by NSURLCache instead of SDWebImage leading to slight performance degradation.
     * This option helps deal with images changing behind the same request URL, e.g. Facebook graph api profile pics.
     * If a cached image is refreshed, the completion block is called once with the cached image and again with the final image.
     *
     * Use this flag only if you can't make your URLs static with embedded cache busting parameter.
     */
    /** 即使图像被高速缓存，但仍然会遵守HTTP响应缓存控制，并根据需要从远程位置刷新图像。
     *  磁盘缓存将会使用NSURLCache代替SDWebImage，这样做会导致轻微的降低性能
     *  该选项有助于应对不断变化的同样请求的URL后面的图像，例如Facebook的图形API的资料照片。
     *  如果缓存图片被刷新，完成的block首先调用内存缓存的图片，然后调用最终的图片
     *  这个标记主要适用于你的非静态URL
     
     * 这个理解会更加确切：这个选项的意思是正常情况下 只会根据url做key去判断图片是否加载过 但是 有些网页是动态网站 图片的url就会变 但是文件的签名 如
        e-tag （http header里面）是用来判断图片是否是同一个 默认sd 没有看http header 如果加上这个标记 就会访问http header .
     */
    SDWebImageRefreshCached = 1 << 4,

    /**
     * In iOS 4+, continue the download of the image if the app goes to background. This is achieved by asking the system for
     * extra time in background to let the request finish. If the background task expires the operation will be cancelled.
     */
    /**允许切入后台继续下载图片。当如果该任务超时会被系统取消。
     
     */
    SDWebImageContinueInBackground = 1 << 5,

    /**
     * Handles cookies stored in NSHTTPCookieStore by setting
     * NSMutableURLRequest.HTTPShouldHandleCookies = YES;
     */
    /** 通过设置NSMutableURLRequest.HTTPShouldHandleCookies = YES来处理cookies在NSHTTPCookieStore的存储问题。
     *  具体用法还希望大神解释
     */
    SDWebImageHandleCookies = 1 << 6,

    /**
     * Enable to allow untrusted SSL certificates.
     * Useful for testing purposes. Use with caution in production.
     */
    /**
     *  允许使用不安全的SSL证书，一般用于测试
     */
    SDWebImageAllowInvalidSSLCertificates = 1 << 7,

    /**
     * By default, image are loaded in the order they were queued. This flag move them to
     * the front of the queue and is loaded immediately instead of waiting for the current queue to be loaded (which 
     * could take a while).
     */
    /**
     *  默认情况下图片会根据在队列里面的顺序加载，这个标记可以把它们一道队列的最前面，然后立马开始加载。
     */
    SDWebImageHighPriority = 1 << 8,
    
    /**
     * By default, placeholder images are loaded while the image is loading. This flag will delay the loading
     * of the placeholder image until after the image has finished loading.
     */
    /**
     *  默认情况下，占位图在图片下载的时候就会被加载。这个标记主要用于延缓占位图加载，等到图片下载完成才开始加载。
     *  不明白这个有什么用？可能是为了减少占位图的显示时间。展位图显示，然后立马被下载的图替换。
     */
    SDWebImageDelayPlaceholder = 1 << 9,

    /**
     * We usually don't call transformDownloadedImage delegate method on animated images,
     * as most transformation code would mangle it.
     * Use this flag to transform them anyway.
     */
    /**
     *  我们通常不对动画图像调用transformdownloadedimage代理方法，大多数的转换代码会破坏图片。用这个标志来改变它们。
     *  没有具体的用过，但我觉得还需要实现transformdownloadedimage相关的其他方法
     */
    SDWebImageTransformAnimatedImage = 1 << 10,
    
    /**
     * By default, image is added to the imageView after download. But in some cases, we want to
     * have the hand before setting the image (apply a filter or add it with cross-fade animation for instance)
     * Use this flag if you want to manually set the image in the completion when success
     */
    /**
     *  默认情况下，图像下载完成之后就会加载到图片上。但有时候我们想在加载在图片之前处理图像（比如说是滤镜等）
     *  如果你想在图像下载完成后手动的修改这张图片的属性，可以使用这个标记
     */
    SDWebImageAvoidAutoSetImage = 1 << 11
};

typedef void(^SDWebImageCompletionBlock)(UIImage *image, NSError *error, SDImageCacheType cacheType, NSURL *imageURL);

typedef void(^SDWebImageCompletionWithFinishedBlock)(UIImage *image, NSError *error, SDImageCacheType cacheType, BOOL finished, NSURL *imageURL);

typedef NSString *(^SDWebImageCacheKeyFilterBlock)(NSURL *url);


@class SDWebImageManager;

@protocol SDWebImageManagerDelegate <NSObject>

@optional

/**
 * Controls which image should be downloaded when the image is not found in the cache.
 *
 * @param imageManager The current `SDWebImageManager`
 * @param imageURL     The url of the image to be downloaded
 *
 * @return Return NO to prevent the downloading of the image on cache misses. If not implemented, YES is implied.
 */
/*
 * 当图片不在缓存的时候，控制将要下载哪一张图像
 */
- (BOOL)imageManager:(SDWebImageManager *)imageManager shouldDownloadImageForURL:(NSURL *)imageURL;

/**
 * Allows to transform the image immediately after it has been downloaded and just before to cache it on disk and memory.
 * NOTE: This method is called from a global queue in order to not to block the main thread.
 *
 * @param imageManager The current `SDWebImageManager`
 * @param image        The image to transform
 * @param imageURL     The url of the image to transform
 *
 * @return The transformed image object.
 */

/*
 *  当图片下载完成之后，在缓存到硬盘和内存之前，允许转换图片
 *  该方法在异步线程中调用，防止阻塞主线程
 */
- (UIImage *)imageManager:(SDWebImageManager *)imageManager transformDownloadedImage:(UIImage *)image withURL:(NSURL *)imageURL;

@end

/**
 * The SDWebImageManager is the class behind the UIImageView+WebCache category and likes.
 * It ties the asynchronous downloader (SDWebImageDownloader) with the image cache store (SDImageCache).
 * You can use this class directly to benefit from web image downloading with caching in another context than
 * a UIView.
 *
 * Here is a simple example of how to use SDWebImageManager:
 *
 * @code

SDWebImageManager *manager = [SDWebImageManager sharedManager];
[manager downloadImageWithURL:imageURL
                      options:0
                     progress:nil
                    completed:^(UIImage *image, NSError *error, SDImageCacheType cacheType, BOOL finished, NSURL *imageURL) {
                        if (image) {
                            // do something with image
                        }
                    }];

 * @endcode
 */
@interface SDWebImageManager : NSObject

@property (weak, nonatomic) id <SDWebImageManagerDelegate> delegate;

@property (strong, nonatomic, readonly) SDImageCache *imageCache;
@property (strong, nonatomic, readonly) SDWebImageDownloader *imageDownloader;

/**
 * The cache filter is a block used each time SDWebImageManager need to convert an URL into a cache key. This can
 * be used to remove dynamic part of an image URL.
 *
 * The following example sets a filter in the application delegate that will remove any query-string from the
 * URL before to use it as a cache key:
 *
 * @code

[[SDWebImageManager sharedManager] setCacheKeyFilter:^(NSURL *url) {
    url = [[NSURL alloc] initWithScheme:url.scheme host:url.host path:url.path];
    return [url absoluteString];
}];

 * @endcode
 */
@property (nonatomic, copy) SDWebImageCacheKeyFilterBlock cacheKeyFilter;

/**
 * Returns global SDWebImageManager instance.
 *
 * @return SDWebImageManager shared instance
 */
+ (SDWebImageManager *)sharedManager;

/**
 * Downloads the image at the given URL if not present in cache or return the cached version otherwise.
 *
 * @param url            The URL to the image
 * @param options        A mask to specify options to use for this request
 * @param progressBlock  A block called while image is downloading
 * @param completedBlock A block called when operation has been completed.
 *
 *   This parameter is required.
 * 
 *   This block has no return value and takes the requested UIImage as first parameter.
 *   In case of error the image parameter is nil and the second parameter may contain an NSError.
 *
 *   The third parameter is an `SDImageCacheType` enum indicating if the image was retrieved from the local cache
 *   or from the memory cache or from the network.
 *
 *   The last parameter is set to NO when the SDWebImageProgressiveDownload option is used and the image is 
 *   downloading. This block is thus called repeatedly with a partial image. When image is fully downloaded, the
 *   block is called a last time with the full image and the last parameter set to YES.
 *
 * @return Returns an NSObject conforming to SDWebImageOperation. Should be an instance of SDWebImageDownloaderOperation
 */
- (id <SDWebImageOperation>)downloadImageWithURL:(NSURL *)url
                                         options:(SDWebImageOptions)options
                                        progress:(SDWebImageDownloaderProgressBlock)progressBlock
                                       completed:(SDWebImageCompletionWithFinishedBlock)completedBlock;

/**
 * Saves image to cache for given URL
 *
 * @param image The image to cache
 * @param url   The URL to the image
 *
 */

- (void)saveImageToCache:(UIImage *)image forURL:(NSURL *)url;

/**
 * Cancel all current operations
 */
- (void)cancelAll;

/**
 * Check one or more operations running
 */
- (BOOL)isRunning;

/**
 *  Check if image has already been cached
 *
 *  @param url image url
 *
 *  @return if the image was already cached
 */
- (BOOL)cachedImageExistsForURL:(NSURL *)url;

/**
 *  Check if image has already been cached on disk only
 *
 *  @param url image url
 *
 *  @return if the image was already cached (disk only)
 */
- (BOOL)diskImageExistsForURL:(NSURL *)url;

/**
 *  Async check if image has already been cached
 *
 *  @param url              image url
 *  @param completionBlock  the block to be executed when the check is finished
 *  
 *  @note the completion block is always executed on the main queue
 */
- (void)cachedImageExistsForURL:(NSURL *)url
                     completion:(SDWebImageCheckCacheCompletionBlock)completionBlock;

/**
 *  Async check if image has already been cached on disk only
 *
 *  @param url              image url
 *  @param completionBlock  the block to be executed when the check is finished
 *
 *  @note the completion block is always executed on the main queue
 */
- (void)diskImageExistsForURL:(NSURL *)url
                   completion:(SDWebImageCheckCacheCompletionBlock)completionBlock;


/**
 *Return the cache key for a given URL
 */
- (NSString *)cacheKeyForURL:(NSURL *)url;

@end


#pragma mark - Deprecated

typedef void(^SDWebImageCompletedBlock)(UIImage *image, NSError *error, SDImageCacheType cacheType) __deprecated_msg("Block type deprecated. Use `SDWebImageCompletionBlock`");
typedef void(^SDWebImageCompletedWithFinishedBlock)(UIImage *image, NSError *error, SDImageCacheType cacheType, BOOL finished) __deprecated_msg("Block type deprecated. Use `SDWebImageCompletionWithFinishedBlock`");


@interface SDWebImageManager (Deprecated)

/**
 *  Downloads the image at the given URL if not present in cache or return the cached version otherwise.
 *
 *  @deprecated This method has been deprecated. Use `downloadImageWithURL:options:progress:completed:`
 */
- (id <SDWebImageOperation>)downloadWithURL:(NSURL *)url
                                    options:(SDWebImageOptions)options
                                   progress:(SDWebImageDownloaderProgressBlock)progressBlock
                                  completed:(SDWebImageCompletedWithFinishedBlock)completedBlock __deprecated_msg("Method deprecated. Use `downloadImageWithURL:options:progress:completed:`");

@end
