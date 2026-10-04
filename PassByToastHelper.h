#ifndef PASSBYTOASTHELPER_H
#define PASSBYTOASTHELPER_H

// Focus-style pill toast, drawn in its own top-level window so it survives
// lock screen transitions. Main thread only.

static const CGFloat        PASSBY_TOAST_HEIGHT     = 40.0;
static const NSTimeInterval PASSBY_TOAST_DURATION   = 2.0;

// Lock screen only renders windows backed by a secure context
@interface PassByToastWindow : UIWindow
@end

@implementation PassByToastWindow
- (BOOL)_shouldCreateContextAsSecure
{
    return YES;
}
@end

static UIWindow * toastWindow = nil;

static void hideToastWindow()
{
    if (toastWindow) {
        toastWindow.hidden = YES;
        [toastWindow release];
        toastWindow = nil;
    }
}

static UIWindow * findKeyWindow()
{
    UIWindow * fallback = nil;
    for (UIScene * scene in [[UIApplication sharedApplication] connectedScenes]) {
        if (![scene isKindOfClass:[UIWindowScene class]])
            continue;
        for (UIWindow * window in [(UIWindowScene *)scene windows]) {
            if ([window isKeyWindow])
                return window;
            if (!fallback)
                fallback = window;
        }
    }
    return fallback;
}

static void showToast(NSString * symbolName, NSString * text)
{
    hideToastWindow();

    UIWindow * keyWindow    = findKeyWindow();
    UIWindowScene * scene   = keyWindow.windowScene;

    toastWindow = scene
        ? [[PassByToastWindow alloc] initWithWindowScene:scene]
        : [[PassByToastWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    toastWindow.windowLevel             = UIWindowLevelStatusBar + 1000;
    toastWindow.userInteractionEnabled  = NO;
    toastWindow.backgroundColor         = [UIColor clearColor];

    // Pill: blurred capsule with icon + label
    UIVisualEffectView * pill =
        [   [UIVisualEffectView alloc]
            initWithEffect:[UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThickMaterial]
        ];
    pill.layer.cornerRadius = PASSBY_TOAST_HEIGHT / 2;
    pill.layer.cornerCurve  = kCACornerCurveContinuous;
    pill.clipsToBounds      = YES;

    UIImageSymbolConfiguration * symbolConfig =
        [UIImageSymbolConfiguration
            configurationWithPointSize:15
            weight:UIImageSymbolWeightSemibold
        ];
    UIImageView * icon =
        [   [UIImageView alloc]
            initWithImage:[UIImage systemImageNamed:symbolName withConfiguration:symbolConfig]
        ];
    icon.tintColor      = [UIColor systemBlueColor];
    icon.contentMode    = UIViewContentModeCenter;

    UILabel * label = [UILabel new];
    label.text      = text;
    label.textColor = [UIColor labelColor];
    label.font      = [UIFont systemFontOfSize:15 weight:UIFontWeightSemibold];

    UIStackView * stack =
        [   [UIStackView alloc]
            initWithArrangedSubviews:@[icon, label]
        ];
    stack.axis      = UILayoutConstraintAxisHorizontal;
    stack.alignment = UIStackViewAlignmentCenter;
    stack.spacing   = 8;
    stack.translatesAutoresizingMaskIntoConstraints = NO;
    [pill.contentView addSubview:stack];

    pill.translatesAutoresizingMaskIntoConstraints = NO;
    [toastWindow addSubview:pill];

    // Sit just below the status bar / Dynamic Island
    CGFloat topInset = keyWindow ? keyWindow.safeAreaInsets.top : 20;
    if (topInset < 20)
        topInset = 20;

    [NSLayoutConstraint activateConstraints:@[
        [pill.centerXAnchor constraintEqualToAnchor:toastWindow.centerXAnchor],
        [pill.topAnchor     constraintEqualToAnchor:toastWindow.topAnchor constant:topInset + 6],
        [pill.heightAnchor  constraintEqualToConstant:PASSBY_TOAST_HEIGHT],
        [stack.leadingAnchor    constraintEqualToAnchor:pill.contentView.leadingAnchor  constant:16],
        [stack.trailingAnchor   constraintEqualToAnchor:pill.contentView.trailingAnchor constant:-18],
        [stack.centerYAnchor    constraintEqualToAnchor:pill.contentView.centerYAnchor],
    ]];

    [icon   release];
    [label  release];
    [stack  release];

    toastWindow.hidden = NO;
    [toastWindow layoutIfNeeded];

    pill.alpha      = 0;
    pill.transform  = CGAffineTransformConcat(
        CGAffineTransformMakeScale(0.6, 0.6),
        CGAffineTransformMakeTranslation(0, -PASSBY_TOAST_HEIGHT)
    );

    UIWindow * window = toastWindow;
    [UIView
        animateWithDuration:0.5
        delay:0
        usingSpringWithDamping:0.75
        initialSpringVelocity:0
        options:0
        animations:^{
            pill.alpha      = 1;
            pill.transform  = CGAffineTransformIdentity;
        }
        completion:^(BOOL) {
            [UIView
                animateWithDuration:0.3
                delay:PASSBY_TOAST_DURATION
                options:UIViewAnimationOptionCurveEaseIn
                animations:^{
                    pill.alpha      = 0;
                    pill.transform  = CGAffineTransformMakeScale(0.8, 0.8);
                }
                completion:^(BOOL) {
                    // A newer toast may have replaced this window meanwhile
                    if (toastWindow == window)
                        hideToastWindow();
                }
            ];
        }
    ];

    [pill release];
}


#else
#error "File already included"
#endif // PASSBYTOASTHELPER_H
