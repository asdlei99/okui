/**
* Copyright 2017 BitTorrent Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <okui/View.h>

#include <okui/Application.h>
#include <okui/BitmapFont.h>
#include <okui/blending.h>
#include <okui/opengl/opengl.h>
#include <okui/shapes/Rectangle.h>
#include <okui/Window.h>

#include <scraps/Reverse.h>

#include <cassert>

namespace okui {

View::~View() {
    if (application()) {
        application()->removeListeners(this);
    }

    if (_nextFocus) {
        _nextFocus->_previousFocus = _previousFocus;
    }

    if (_previousFocus) {
        _previousFocus->_nextFocus = _nextFocus;
    }

    removeSubviews();

    if (superview()) {
        superview()->removeSubview(this);
    }
}

void View::addSubview(View* view) {
    assert(view != this);

    if (view->_name.empty()) {
        view->setName(view->name()); // name() returns typeid if _name is empty
    }

    bool viewIsAppearing = !view->isVisibleInOpenWindow() && view->isVisible() && isVisibleInOpenWindow();

    if (viewIsAppearing) {
        view->_dispatchFutureVisibilityChange(true);
    }

    if (view->superview()) {
        view->superview()->removeSubview(view);
    }

    addChildToFront(view);

    assert(view->window() == nullptr);
    if (_window && _window->isOpen()) {
        view->_dispatchWindowChange(_window);
    }

    if (viewIsAppearing) {
        view->_dispatchVisibilityChange(true);
    }

    invalidateRenderCache();
}

void View::addHiddenSubview(View* view) {
    view->hide();
    addSubview(view);
}

void View::removeSubview(View* view) {
    assert(view != this);

    if (view->isFocus()) {
        view->focusAncestor();
    }

    bool viewIsDisappearing = view->isVisibleInOpenWindow();

    if (viewIsDisappearing) {
        view->_dispatchFutureVisibilityChange(false);
    }

    if (_subviewWithMouse == view) {
        _subviewWithMouse = nullptr;
    }

    removeChild(view);

    if (view->window() != nullptr) {
        view->_dispatchWindowChange(nullptr);
    }

    if (viewIsDisappearing) {
        view->_dispatchVisibilityChange(false);
    }

    invalidateRenderCache();
}

void View::removeSubviews() {
    while (!subviews().empty()) {
        removeSubview(subviews().front());
    }
}

void View::setIsVisible(bool isVisible) {
    if (_isVisible == isVisible) { return; }

    if (superview() && superview()->isVisibleInOpenWindow()) {
        _dispatchFutureVisibilityChange(isVisible);
    }

    _isVisible = isVisible;

    if (superview() && superview()->isVisibleInOpenWindow()) {
        _dispatchVisibilityChange(isVisible);
    }

    if (superview()) {
        superview()->invalidateRenderCache();
    }
}

bool View::ancestorsAreVisible() const {
    return !superview() || (superview()->isVisible() && superview()->ancestorsAreVisible());
}

bool View::isVisibleInOpenWindow() const {
    return isVisible() && ancestorsAreVisible() && window() && window()->isOpen();
}

void View::setInterceptsInteractions(bool intercepts) {
    _interceptsInteractions = intercepts;
}

void View::setInterceptsInteractions(bool intercepts, bool childrenIntercept) {
    _interceptsInteractions = intercepts;
    _childrenInterceptInteractions = childrenIntercept;
}

void View::setChildrenInterceptInteractions(bool childrenIntercept) {
    _childrenInterceptInteractions = childrenIntercept;
}

ShaderCache* View::shaderCache() {
    return window()->shaderCache();
}

void View::setScale(double scaleX, double scaleY) {
    if (_scale.x == scaleX && _scale.y == scaleY) { return; }
    _scale.x = scaleX;
    _scale.y = scaleY;
    invalidateRenderCache();
}

void View::setTintColor(const Color& color) {
    if (_tintColor == color) { return; }
    _tintColor = color;
    _invalidateSuperviewRenderCache();
}

void View::sendToBack() {
    TreeNode::sendToBack();
    if (isVisible()) {
        superview()->invalidateRenderCache();
    }
}

void View::bringToFront() {
    TreeNode::bringToFront();
    if (isVisible()) {
        superview()->invalidateRenderCache();
    }
}

void View::focus() {
    if (window()) {
        window()->setFocus(this);
    }
}

View* View::expectedFocus() {
    auto view = this;
    View* ret = nullptr;
    while (view) {
        if (view->canBecomeDirectFocus() && view->isVisibleInOpenWindow()) {
            ret = view;
        }
        view = view->preferredFocus();
    }
    return ret;
}

void View::focusAncestor() {
    for (auto ancestor = superview(); ancestor; ancestor = ancestor->superview()) {
        auto focus = ancestor->expectedFocus();
        if (focus && focus != this && !focus->isDescendantOf(this)) {
            focus->focus();
            return;
        }
    }

    unfocus();
}

void View::unfocus() {
    if (isFocus()) {
        window()->setFocus(nullptr);
    }
}

void View::setNextFocus(View* view) {
    if (_nextFocus == view) { return; }

    if (_nextFocus) {
        if (_nextFocus->_previousFocus && _nextFocus->_previousFocus != this) {
            _nextFocus->_previousFocus->_nextFocus = nullptr;
        }
        _nextFocus->_previousFocus = nullptr;
    }

    _nextFocus = view;

    if (_nextFocus) {
        _nextFocus->_previousFocus = this;
    }
}

View* View::nextAvailableFocus() {
    auto view = _nextFocus;
    while (view && view != this) {
        if (view->isVisible() && view->canBecomeDirectFocus()) {
            return view;
        }
        view = view->_nextFocus;
    }
    return nullptr;
}

View* View::previousAvailableFocus() {
    auto view = _previousFocus;
    while (view && view != this) {
        if (view->isVisible() && view->canBecomeDirectFocus()) {
            return view;
        }
        view = view->_previousFocus;
    }
    return nullptr;
}

bool View::isFocus() const {
    return window() && window()->focus() && (window()->focus() == this || window()->focus()->isDescendantOf(this));
}

bool View::hasMouse() const {
    return superview() && superview()->_subviewWithMouse == this;
}

std::shared_ptr<TextureInterface> View::renderTexture() const {
    return _renderCacheTexture;
}

void View::invalidateRenderCache() {
    _hasCachedRender = false;
    _invalidateSuperviewRenderCache();
}

TextureHandle View::loadTextureResource(const std::string& name) {
    if (!window()) { return nullptr; }
    auto handle = window()->loadTextureResource(name);
    handle.onLoad([this]{ invalidateRenderCache(); });
    return handle;
}

TextureHandle View::loadTextureFromMemory(std::shared_ptr<const std::string> data) {
    if (!window()) { return nullptr; }
    auto handle = window()->loadTextureFromMemory(data);
    handle.onLoad([this]{ invalidateRenderCache(); });
    return handle;
}

TextureHandle View::loadTextureFromURL(const std::string& url) {
    if (!window()) { return nullptr; }
    auto handle = window()->loadTextureFromURL(url);
    handle.onLoad([this]{ invalidateRenderCache(); });
    return handle;
}

Application* View::application() const {
    return window() ? window()->application() : nullptr;
}

Rectangle<double> View::windowBounds() const {
    auto min = localToWindow(0, 0);
    auto max = localToWindow(bounds().width, bounds().height);
    return Rectangle<double>(min, max - min);
}

void View::setBoundsRelative(double x, double y, double width, double height) {
    if (superview()) {
        auto& superBounds = superview()->bounds();
        setBounds(x * superBounds.width, y * superBounds.height, width * superBounds.width, height * superBounds.height);
    }
}

Point<double> View::localToAncestor(double x, double y, const View* ancestor) const {
    assert(!ancestor || ancestor->isAncestorOf(this));
    const auto xSuper = bounds().x + x;
    const auto ySuper = bounds().y + y;
    return ancestor && ancestor != superview() ? superview()->localToAncestor(xSuper, ySuper, ancestor) : Point<double>(xSuper, ySuper);
}

Point<double> View::superviewToLocal(double x, double y) const {
    return Point<double>(x - bounds().x, y - bounds().y);
}

Point<double> View::superviewToLocal(const Point<double>& p) const {
    return superviewToLocal(p.x, p.y);
}

Point<double> View::localToWindow(double x, double y) const {
    Point<double> ret(x, y);
    auto view = this;
    while (view) {
        ret = view->localToSuperview(ret);
        view = view->superview();
    }
    return ret;
}

Point<double> View::localToWindow(const Point<double>& p) const {
    return localToWindow(p.x, p.y);
}

bool View::hitTest(double x, double y) {
    return x >= 0 && x < bounds().width &&
           y >= 0 && y < bounds().height;
}

View* View::hitTestView(double x, double y) {
    auto hit = hitTest(x, y);
    if (hit || !_clipsToBounds) {
        for (auto& subview : subviews()) {
            if (subview->isVisible()) {
                auto point = subview->superviewToLocal(x, y);
                auto view = subview->hitTestView(point.x, point.y);
                if (view) { return view; }
            }
        }
    }

    return hit ? this : nullptr;
}

void View::mouseDown(MouseButton button, double x, double y) {
    if (superview() && superview()->_interceptsInteractions) {
        auto point = localToSuperview(x, y);
        superview()->mouseDown(button, point.x, point.y);
        window()->beginDragging(superview());
    }
}

void View::mouseUp(MouseButton button, double startX, double startY, double x, double y) {
    if (superview() && superview()->_interceptsInteractions) {
        auto startPoint = localToSuperview(startX, startY);
        auto point = localToSuperview(x, y);
        superview()->mouseUp(button, startPoint.x, startPoint.y, point.x, point.y);
    }
}

void View::mouseWheel(double xPos, double yPos, int xWheel, int yWheel) {
    if (superview() && superview()->_interceptsInteractions) {
        auto point = localToSuperview(xPos, yPos);
        superview()->mouseWheel(point.x, point.y, xWheel, yWheel);
    }
}

void View::mouseDrag(double startX, double startY, double x, double y) {
    if (superview() && superview()->_interceptsInteractions) {
        auto startPoint = localToSuperview(startX, startY);
        auto point = localToSuperview(x, y);
        superview()->mouseDrag(startPoint.x, startPoint.y, point.x, point.y);
    }
}

void View::mouseMovement(double x, double y) {
    if (superview() && superview()->_interceptsInteractions) {
        auto point = localToSuperview(x, y);
        superview()->mouseMovement(point.x, point.y);
    }
}

Responder* View::nextResponder() {
    return superview() ? dynamic_cast<Responder*>(superview()) : dynamic_cast<Responder*>(window());
}

void View::keyDown(KeyCode key, KeyModifiers mod, bool repeat) {
    if (key == KeyCode::kTab) {
        if (_previousFocus && (mod & KeyModifier::kShift)) {
            if (auto view = previousAvailableFocus()) {
                view->focus();
            }
            return;
        } else if (_nextFocus) {
            if (auto view = nextAvailableFocus()) {
                view->focus();
            }
            return;
        }
    }

    if (window() && (false
        || (key == KeyCode::kRight && window()->moveFocus(Direction::kRight))
        || (key == KeyCode::kLeft && window()->moveFocus(Direction::kLeft))
        || (key == KeyCode::kUp && window()->moveFocus(Direction::kUp))
        || (key == KeyCode::kDown && window()->moveFocus(Direction::kDown))
    )) {
        return;
    }

    Responder::keyDown(key, mod, repeat);
}

bool View::hasRelation(Relation relation, const View* view) const {
    switch (relation) {
        case Relation::kAny:
            return application() ? application() == view->application() : commonView(view) != nullptr;
        case Relation::kHierarchy:
            return TreeNode::hasRelation(TreeNode::Relation::kCommonRoot, view);
        case Relation::kDescendant:
            return TreeNode::hasRelation(TreeNode::Relation::kDescendant, view);
        case Relation::kAncestor:
            return TreeNode::hasRelation(TreeNode::Relation::kAncestor, view);
        case Relation::kSibling:
            return TreeNode::hasRelation(TreeNode::Relation::kSibling, view);
        case Relation::kSelf:
            return TreeNode::hasRelation(TreeNode::Relation::kSelf, view);
    }
}

void View::dispatchUpdate(std::chrono::high_resolution_clock::duration elapsed) {
    if (!_shouldSubscribeToUpdates()) {
        window()->unsubscribeFromUpdates(this);
        return;
    }

    if (scraps::platform::kIsTVOS && canBecomeDirectFocus()) {
        View* newFocus = nullptr;
        _touchpadFocus.update(elapsed, this, &newFocus);
        if (newFocus) {
            newFocus->_checkUpdateSubscription();
        }
    }

    auto hooks = _updateHooks;
    for (auto& hook : hooks) {
        if (_updateHooks.count(hook.first)) {
            hook.second();
        }
    }
}

void View::renderAndRenderSubviews(const RenderTarget* target, const Rectangle<int>& area, stdts::optional<Rectangle<int>> clipBounds) {
    if (!isVisible() || !area.width || !area.height) { return; }

    if (!_requiresTextureRendering() && !_cachesRender) {
        // render directly
        _renderAndRenderSubviews(target, area, false, clipBounds);
        _renderCacheTexture->set();
        return;
    }

    clipBounds = clipBounds ? clipBounds->intersection(area) : area;

    if (!_rendersToTexture && clipBounds->size().magnitudeSquared() == 0) {
        return;
    }

    // make sure the render cache is up-to-date

    GLint previousFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    if (!_renderCacheColorAttachment || _renderCacheColorAttachment->width() != area.width || _renderCacheColorAttachment->height() != area.height) {
        // update the framebuffer
        _renderCache = std::make_unique<opengl::Framebuffer>();
        _renderCacheColorAttachment = _renderCache->addColorAttachment(area.width, area.height);
        // TODO: stencil attachment
        assert(_renderCache->isComplete());
        _hasCachedRender = false;
    }
    _renderCacheTexture->set(_renderCacheColorAttachment->texture(), area.width, area.height, true);

    if (!_cachesRender || !_hasCachedRender) {
        // render to _renderCache
        _renderCache->bind();
        Rectangle<int> cacheArea(0, 0, area.width, area.height);
        RenderTarget cacheTarget(area.width, area.height);
        _renderAndRenderSubviews(&cacheTarget, cacheArea, true);
        _hasCachedRender = true;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);

    // do the actual rendering

    glViewport(area.x, target->height() - area.maxY(), area.width, area.height);
    Blending blending{BlendFunction::kDefault};
    glEnable(GL_SCISSOR_TEST);
    glScissor(clipBounds->x, target->height() - clipBounds->maxY(), clipBounds->width, clipBounds->height);

    AffineTransformation transformation{-1, -1, 0, 0, 2.0/_bounds.width, 2.0/_bounds.height};
    postRender(_renderCacheTexture, transformation);

    glDisable(GL_SCISSOR_TEST);
}

void View::addUpdateHook(const std::string& handle, std::function<void()> hook) {
    _updateHooks[std::hash<std::string>()(handle)] = std::move(hook);
    _checkUpdateSubscription();
}

void View::removeUpdateHook(const std::string& handle) {
    _updateHooks.erase(std::hash<std::string>()(handle));
    _checkUpdateSubscription();
}

void View::postRender(std::shared_ptr<TextureInterface> texture, const AffineTransformation& transformation) {
    auto shader = textureShader();
    shader->setTransformation(transformation);
    shader->setColor(_tintColor);
    shader->drawScaledFill(*texture, Rectangle<double>(0.0, 0.0, bounds().width, bounds().height));
    shader->flush();
}

bool View::dispatchMouseDown(MouseButton button, double x, double y) {
    if (!isVisible()) { return false; }

    if (_childrenInterceptInteractions) {
        for (auto& subview : subviews()) {
            auto point = subview->superviewToLocal(x, y);
            if ((!subview->clipsToBounds() || subview->hitTest(point.x, point.y)) && subview->dispatchMouseDown(button, point.x, point.y)) {
                return true;
            }
        }
    }
    if (_interceptsInteractions && hitTest(x, y)) {
        mouseDown(button, x, y);
        window()->beginDragging(this);
        return true;
    }

    return false;
}

bool View::dispatchMouseUp(MouseButton button, double startX, double startY, double x, double y) {
    if (!isVisible()) { return false; }

    if (_childrenInterceptInteractions) {
        for (auto& subview : subviews()) {
            auto startPoint = subview->superviewToLocal(startX, startY);
            auto point = subview->superviewToLocal(x, y);
            if ((!subview->clipsToBounds() || subview->hitTest(point.x, point.y)) && subview->dispatchMouseUp(button, startPoint.x, startPoint.y, point.x, point.y)) {
                return true;
            }
        }
    }
    if (_interceptsInteractions && hitTest(x, y)) {
        mouseUp(button, startX, startY, x, y);
        return true;
    }

    return false;
}

bool View::dispatchMouseMovement(double x, double y) {
    if (!isVisible()) { return false; }

    View* subviewWithMouse = nullptr;

    if (_childrenInterceptInteractions) {
        for (auto& subview : subviews()) {
            auto point = subview->superviewToLocal(x, y);
            if ((!subview->clipsToBounds() || subview->hitTest(point.x, point.y)) && subview->dispatchMouseMovement(point.x, point.y)) {
                subviewWithMouse = subview;
                break;
            }
        }
    }

    if (subviewWithMouse != _subviewWithMouse) {
        if (_subviewWithMouse) {
            _subviewWithMouse->_mouseExit();
        }
        _subviewWithMouse = subviewWithMouse;
        if (_subviewWithMouse) {
            _subviewWithMouse->mouseEnter();
        }
    }

    if (subviewWithMouse) {
        return true;
    }

    if (_interceptsInteractions && hitTest(x, y)) {
        mouseMovement(x, y);
        return true;
    }

    return false;
}

bool View::dispatchMouseWheel(double xPos, double yPos, int xWheel, int yWheel) {
    if (!isVisible()) { return false; }

    if (_childrenInterceptInteractions) {
        for (auto& subview : subviews()) {
            auto point = subview->superviewToLocal(xPos, yPos);
            if ((!subview->clipsToBounds() || subview->hitTest(point.x, point.y)) &&
                subview->dispatchMouseWheel(point.x, point.y, xWheel, yWheel)) {
                return true;
            }
        }
    }

    if (_interceptsInteractions && hitTest(xPos, yPos)) {
        mouseWheel(xPos, yPos, xWheel, yWheel);
        return true;
    }

    return false;
}

void View::touchUp(size_t finger, Point<double> position, double pressure) {
    if (!scraps::platform::kIsTVOS) { return; }
    _touchpadFocus.touchUp(position);
    _checkUpdateSubscription();
}

void View::touchDown(size_t finger, Point<double> position, double pressure) {
    if (!scraps::platform::kIsTVOS) { return; }
    _touchpadFocus.touchDown(position);
    _checkUpdateSubscription();
}

void View::touchMovement(size_t finger, Point<double> position, Point<double> distance, double pressure) {
    if (!scraps::platform::kIsTVOS) { return; }
    _touchpadFocus.touchMovement(position, distance);
    _checkUpdateSubscription();
}

void View::_invalidateSuperviewRenderCache() {
    if (isVisible() && superview()) {
        superview()->invalidateRenderCache();
    }
}

void View::_setBounds(const Rectangle<double>& bounds) {
    auto willMove = (_bounds.x != bounds.x || _bounds.y != bounds.y);
    auto willResize = (_bounds.width != bounds.width || _bounds.height != bounds.height);

    if (!willMove && !willResize) {
        return;
    }

    _bounds = std::move(bounds);

    if (willResize) {
        layout();
        invalidateRenderCache();
    }

    if (isVisible() && superview()) {
        superview()->invalidateRenderCache();
    }
}

void View::_dispatchFutureVisibilityChange(bool visible) {
    if (visible) {
        willAppear();
    } else {
        willDisappear();
    }

    for (auto& subview : subviews()) {
        if (subview->isVisible()) {
            subview->_dispatchFutureVisibilityChange(visible);
        }
    }
}

void View::_dispatchVisibilityChange(bool visible) {
    if (!visible && isFocus()) {
        focusAncestor();
    }

    if (visible) {
        appeared();
    } else {
        disappeared();
    }

    for (auto& subview : subviews()) {
        if (subview->isVisible()) {
            subview->_dispatchVisibilityChange(visible);
        }
    }

    _checkUpdateSubscription();
}

void View::_dispatchWindowChange(Window* window) {
    if (application()) {
        application()->removeListeners(this);
    }

    if (_window) {
        _window->endDragging(this);
        _window->unsubscribeFromUpdates(this);
    }

    assert(_window != window);
    _window = window;

    if (application()) {
        for (auto& listener : _listeners) {
            application()->addListener(this, listener.index, &listener.action, listener.relation);
        }
    }

    windowChanged();

    for (auto& subview : subviews()) {
        if (window == subview->window()) {
            // subviews added in the above windowChanged call will have already gotten the change
            continue;
        }
        subview->_dispatchWindowChange(window);
    }

    _checkUpdateSubscription();
}

void View::_mouseExit() {
    if (_subviewWithMouse && _clipsToBounds) {
        _subviewWithMouse->_mouseExit();
        _subviewWithMouse = nullptr;
    }
    mouseExit();
}

void View::_updateFocusableRegions(std::vector<std::tuple<View*, Rectangle<double>>>& regions) {
    if (!isVisible() || !window()) { return; }

    if (_interceptsInteractions) {
        auto windowBounds = this->windowBounds();
        std::vector<std::tuple<View*, Rectangle<double>>> prev;
        prev.swap(regions);
        for (auto& region : prev) {
            auto diff = std::get<Rectangle<double>>(region) - windowBounds;
            for (auto& r : diff) {
                regions.emplace_back(std::get<View*>(region), r);
            }
        }
        auto focus = window()->focus();
        if (canBecomeDirectFocus() && (!focus || (focus != this && !isDescendantOf(focus) && !isAncestorOf(focus)))) {
            regions.emplace_back(this, windowBounds);
            if (clipsToBounds()) {
                return;
            }
        }
    }

    if (_childrenInterceptInteractions) {
        for (auto subview : Reverse(subviews())) {
            subview->_updateFocusableRegions(regions);
        }
    }
}

bool View::_requiresTextureRendering() {
    return _rendersToTexture || _tintColor != Color::kWhite;
}

void View::_renderAndRenderSubviews(const RenderTarget* target, const Rectangle<int>& area, bool shouldClear, stdts::optional<Rectangle<int>> clipBounds) {
    auto xScale = (_bounds.width != 0.0 ? area.width / _bounds.width : 1.0);
    auto yScale = (_bounds.height != 0.0 ? area.height / _bounds.height : 1.0);

    auto visibleArea = Rectangle<int>{0, 0, target->width(), target->height()}.intersection(area);
    _renderTransformation = AffineTransformation(
        -1, 1,
        (area.x - visibleArea.x) / xScale, (area.y - visibleArea.y) / yScale,
        2.0/_bounds.width*area.width/visibleArea.width, -2.0/_bounds.height*area.height/visibleArea.height
    );

    if (_clipsToBounds) {
        clipBounds = clipBounds ? clipBounds->intersection(visibleArea) : visibleArea;
        if (clipBounds->size().magnitudeSquared() == 0) {
            return;
        }
    }

    glViewport(visibleArea.x, target->height() - visibleArea.maxY(), visibleArea.width, visibleArea.height);
    Blending blending{BlendFunction::kDefault};

    if (shouldClear) {
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    if (clipBounds) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(clipBounds->x, target->height() - clipBounds->maxY(), clipBounds->width, clipBounds->height);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }

    if (_backgroundColor.alpha() > 0) {
        auto backgroundShader = colorShader();
        backgroundShader->setColor(_backgroundColor);
        shapes::Rectangle(0.0, 0.0, bounds().width, bounds().height).draw(backgroundShader);
        backgroundShader->flush();
    }

    render(target, area);

    for (auto& subview : Reverse(subviews())) {
        Rectangle<int> subarea(std::round(area.x + xScale * subview->_bounds.x),
                               std::round(area.y + yScale * subview->_bounds.y),
                               std::round(xScale * subview->_scale.x * subview->_bounds.width),
                               std::round(yScale * subview->_scale.y * subview->_bounds.height));
        subview->renderAndRenderSubviews(target, subarea, clipBounds);
    }

    if (clipBounds) {
        glDisable(GL_SCISSOR_TEST);
    }
}

void View::_post(std::type_index index, const void* ptr, Relation relation) {
    if (application()) {
        application()->post(this, index, ptr, relation);
    }
}

void View::_listen(std::type_index index, std::function<void(const void*, View*)> action, Relation relation) {
    _listeners.emplace_back(index, std::move(action), relation);
    if (application()) {
        application()->addListener(this, index, &_listeners.back().action, relation);
    }
}

std::vector<View*> View::_topViewsForRelation(Relation relation) {
    auto* thisRootView = root();
    std::vector<View*> ret{thisRootView};
    if (relation == Relation::kAny && application()) {
        for (auto window : application()->windows()) {
            if (window->contentView() != thisRootView) {
                ret.emplace_back(window->contentView());
            }
        }
    }
    return ret;
}

scraps::AbstractTaskScheduler* View::_taskScheduler() const {
    assert(application());
    return application()->taskScheduler();
}

void View::_checkUpdateSubscription() {
    if (!window()) { return; }
    if (_shouldSubscribeToUpdates()) {
        window()->subscribeToUpdates(this);
    } else {
        window()->unsubscribeFromUpdates(this);
    }
}

bool View::_shouldSubscribeToUpdates() {
    return (!_updateHooks.empty() || _touchpadFocus.needsUpdates()) && isVisibleInOpenWindow();
}

} // namespace okui
