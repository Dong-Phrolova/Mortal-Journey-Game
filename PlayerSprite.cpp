#pragma execution_character_set("utf-8")
#include "PlayerSprite.h"
#include "TileSet.h"
#include <cmath>

PlayerSprite::PlayerSprite() {
    m_pixelPos = sf::Vector2f(
        (float)(m_tileX * TileSet::TILE_SIZE),
        (float)(m_tileY * TileSet::TILE_SIZE)
    );
    PreRenderCharacter();
}

void PlayerSprite::SetTilePosition(int tileX, int tileY) {
    m_tileX = tileX;
    m_tileY = tileY;
    m_pixelPos = sf::Vector2f(
        (float)(tileX * TileSet::TILE_SIZE),
        (float)(tileY * TileSet::TILE_SIZE)
    );
    m_moving = false;
}

void PlayerSprite::MoveTo(int tileX, int tileY) {
    if (m_moving) return;
    m_moveFrom = m_pixelPos;
    m_moveTo = sf::Vector2f(
        (float)(tileX * TileSet::TILE_SIZE),
        (float)(tileY * TileSet::TILE_SIZE)
    );
    m_tileX = tileX;
    m_tileY = tileY;
    m_moving = true;
    m_moveProgress = 0.f;
}

void PlayerSprite::Update(float dt) {
    if (!m_moving) {
        m_animTimer -= dt;
        if (m_animTimer <= 0.f) m_animFrame = 0;
        return;
    }

    float dist = sqrtf((m_moveTo.x - m_moveFrom.x)*(m_moveTo.x - m_moveFrom.x) +
                       (m_moveTo.y - m_moveFrom.y)*(m_moveTo.y - m_moveFrom.y));
    if (dist > 0.001f) {
        m_moveProgress += (m_moveSpeed * dt) / dist;
        if (m_moveProgress >= 1.f) {
            m_moveProgress = 1.f;
            m_pixelPos = m_moveTo;
            m_moving = false;
            m_animFrame = 0;
        } else {
            m_pixelPos = m_moveFrom + (m_moveTo - m_moveFrom) * m_moveProgress;
        }
    } else {
        m_pixelPos = m_moveTo;
        m_moving = false;
    }

    m_animTimer += dt;
    const float STEP_INTERVAL = 0.12f;
    if (m_animTimer >= STEP_INTERVAL * 3)
        m_animTimer = 0.f;

    if (m_animTimer > 0 && m_animTimer < STEP_INTERVAL)
        m_animFrame = 1;
    else if (m_animTimer >= STEP_INTERVAL && m_animTimer < STEP_INTERVAL * 2)
        m_animFrame = 0;
    else
        m_animFrame = 2;
}

void PlayerSprite::Render(sf::RenderTarget& target, float cameraX, float cameraY) {
    float px = m_pixelPos.x + cameraX;
    float py = m_pixelPos.y + cameraY;
    DrawCharacter(target, px, py);
}

// ============================================================
//  角色绘制 — 像素风韩立（32x32像素）
//  配色：青色长袍 + 黑色束发 + 肤色
// ============================================================
void PlayerSprite::DrawCharacter(sf::RenderTarget& target, float px, float py) {
    int ts = TileSet::TILE_SIZE;  // 32

    // === 配色 ===
    const sf::Color robe(38, 128, 95);
    const sf::Color robeDark(26, 90, 65);
    const sf::Color robeLight(52, 160, 120);
    const sf::Color robeShadow(20, 70, 50);
    const sf::Color skin(255, 215, 175);
    const sf::Color skinShadow(220, 185, 150);
    const sf::Color hair(28, 22, 18);
    const sf::Color belt(160, 115, 50);
    const sf::Color shoes(50, 45, 38);
    const sf::Color eyeWhite(235, 230, 220);
    const sf::Color eyePupil(25, 20, 18);

    // 行走动画偏移
    float legOff = 0.f, bodyBob = 0.f;
    if (m_moving) {
        switch (m_animFrame) {
        case 1: legOff = -1.0f; bodyBob = -0.5f; break;
        case 2: legOff = 1.0f; bodyBob = -0.5f; break;
        default: break;
        }
    }

    // === 1. 阴影 ===
    {
        sf::RectangleShape sh(sf::Vector2f(14.f, 4.f));
        sh.setPosition(px + 9.f, py + 28.f);
        sh.setFillColor(sf::Color(0, 0, 0, 50));
        target.draw(sh);
    }

    // === 2. 后腿 ===
    {
        bool rev = (m_dir == Dir::Up);
        float bx = rev ? px + 10.f : px + 18.f;
        sf::RectangleShape legR(sf::Vector2f(4.f, 8.f));
        legR.setPosition(bx + legOff, py + 21.f + bodyBob);
        legR.setFillColor(rev ? robe : robeDark);
        target.draw(legR);
        sf::RectangleShape shoeR(sf::Vector2f(5.f, 3.f));
        shoeR.setPosition(bx - 0.5f + legOff, py + 28.f + bodyBob);
        shoeR.setFillColor(shoes);
        target.draw(shoeR);
    }

    // === 3. 身体 ===
    {
        sf::RectangleShape body(sf::Vector2f(14.f, 14.f));
        body.setPosition(px + 9.f, py + 11.f + bodyBob);
        body.setFillColor(robe);
        target.draw(body);
        sf::RectangleShape rd(sf::Vector2f(4.f, 14.f));
        rd.setPosition(px + 19.f, py + 11.f + bodyBob);
        rd.setFillColor(robeDark);
        target.draw(rd);
        sf::RectangleShape hl(sf::Vector2f(2.f, 13.f));
        hl.setPosition(px + 9.f, py + 12.f + bodyBob);
        hl.setFillColor(robeLight);
        target.draw(hl);
        sf::RectangleShape bot(sf::Vector2f(14.f, 2.f));
        bot.setPosition(px + 9.f, py + 23.f + bodyBob);
        bot.setFillColor(robeShadow);
        target.draw(bot);
        // 衣襟交叉
        sf::RectangleShape lapel1(sf::Vector2f(6.f, 6.f));
        lapel1.setPosition(px + 11.f, py + 11.f + bodyBob);
        lapel1.setFillColor(robeDark);
        target.draw(lapel1);
        sf::RectangleShape lapel2(sf::Vector2f(4.f, 4.f));
        lapel2.setPosition(px + 15.f, py + 11.f + bodyBob);
        lapel2.setFillColor(robeLight);
        target.draw(lapel2);
    }

    // === 4. 腰带 ===
    {
        sf::RectangleShape beltRect(sf::Vector2f(14.f, 2.f));
        beltRect.setPosition(px + 9.f, py + 20.f + bodyBob);
        beltRect.setFillColor(belt);
        target.draw(beltRect);
        sf::RectangleShape buckle(sf::Vector2f(3.f, 3.f));
        buckle.setPosition(px + 14.5f, py + 19.5f + bodyBob);
        buckle.setFillColor(sf::Color(200, 175, 80));
        target.draw(buckle);
    }

    // === 5. 前腿 ===
    {
        bool rev = (m_dir == Dir::Up);
        float fx = rev ? px + 18.f : px + 10.f;
        sf::RectangleShape legF(sf::Vector2f(4.f, 8.f));
        legF.setPosition(fx - legOff, py + 21.f + bodyBob);
        legF.setFillColor(rev ? robeDark : robe);
        target.draw(legF);
        sf::RectangleShape shoeF(sf::Vector2f(5.f, 3.f));
        shoeF.setPosition(fx - 0.5f - legOff, py + 28.f + bodyBob);
        shoeF.setFillColor(shoes);
        target.draw(shoeF);
    }

    // === 6. 手臂 ===
    {
        float armOff = (m_moving && m_animFrame == 1) ? 0.5f : 0.f;
        sf::RectangleShape lArm(sf::Vector2f(3.f, 8.f));
        lArm.setPosition(px + 6.f, py + 12.f + bodyBob + armOff);
        lArm.setFillColor(robeDark);
        target.draw(lArm);
        sf::RectangleShape rArm(sf::Vector2f(3.f, 8.f));
        rArm.setPosition(px + 23.f, py + 12.f + bodyBob - armOff);
        rArm.setFillColor(robeDark);
        target.draw(rArm);
    }

    // === 7. 脸 (11x10) ===
    {
        float hx = px + 10.5f, hy = py + 1.f + bodyBob;
        sf::RectangleShape face(sf::Vector2f(11.f, 10.f));
        face.setPosition(hx, hy);
        face.setFillColor(skin);
        target.draw(face);
        // 脸颊阴影
        sf::RectangleShape chkL(sf::Vector2f(2.f, 4.f));
        chkL.setPosition(hx, hy + 4.f);
        chkL.setFillColor(skinShadow);
        target.draw(chkL);
        sf::RectangleShape chkR(sf::Vector2f(2.f, 4.f));
        chkR.setPosition(hx + 9.f, hy + 4.f);
        chkR.setFillColor(skinShadow);
        target.draw(chkR);
    }

    // === 8. 头发 ===
    {
        float hx = px + 10.f, hy = py + bodyBob;
        switch (m_dir) {
        case Dir::Down: {
            sf::RectangleShape top(sf::Vector2f(12.f, 5.f));
            top.setPosition(hx, hy + 1.f);
            top.setFillColor(hair);
            target.draw(top);
            sf::RectangleShape bangs(sf::Vector2f(4.f, 3.f));
            bangs.setPosition(hx + 4.f, hy + 4.f);
            bangs.setFillColor(hair);
            target.draw(bangs);
            sf::RectangleShape sl(sf::Vector2f(2.f, 6.f));
            sl.setPosition(hx - 1.f, hy + 3.f); sl.setFillColor(hair); target.draw(sl);
            sf::RectangleShape sr(sf::Vector2f(2.f, 6.f));
            sr.setPosition(hx + 11.f, hy + 3.f); sr.setFillColor(hair); target.draw(sr);
            sf::RectangleShape bun(sf::Vector2f(5.f, 4.f));
            bun.setPosition(hx + 3.5f, hy - 2.f); bun.setFillColor(hair); target.draw(bun);
            sf::RectangleShape pin(sf::Vector2f(1.5f, 6.f));
            pin.setPosition(hx + 6.75f, hy - 3.f);
            pin.setFillColor(sf::Color(190, 160, 55));
            target.draw(pin);
            break;
        }
        case Dir::Up: {
            sf::RectangleShape bh(sf::Vector2f(11.f, 9.f));
            bh.setPosition(hx, hy + 2.f); bh.setFillColor(hair); target.draw(bh);
            sf::RectangleShape bun2(sf::Vector2f(5.f, 4.f));
            bun2.setPosition(hx + 3.f, hy); bun2.setFillColor(hair); target.draw(bun2);
            break;
        }
        case Dir::Left:
        case Dir::Right: {
            sf::RectangleShape sh(sf::Vector2f(10.f, 7.f));
            sh.setPosition(hx + 1.f, hy + 1.f); sh.setFillColor(hair); target.draw(sh);
            sf::RectangleShape bun3(sf::Vector2f(4.f, 3.f));
            bun3.setPosition(hx + 4.f, hy - 1.f); bun3.setFillColor(hair); target.draw(bun3);
            break;
        }
        }
    }

    // === 9. 面部 ===
    {
        float fy = py + 7.f + bodyBob;
        switch (m_dir) {
        case Dir::Down: {
            float fx = px + 14.f;
            sf::RectangleShape ewl(sf::Vector2f(2.5f, 2.5f));
            ewl.setPosition(fx - 1.f, fy); ewl.setFillColor(eyeWhite); target.draw(ewl);
            sf::RectangleShape ewr(sf::Vector2f(2.5f, 2.5f));
            ewr.setPosition(fx + 6.5f, fy); ewr.setFillColor(eyeWhite); target.draw(ewr);
            sf::RectangleShape epl(sf::Vector2f(1.5f, 1.5f));
            epl.setPosition(fx, fy + 0.5f); epl.setFillColor(eyePupil); target.draw(epl);
            sf::RectangleShape epr(sf::Vector2f(1.5f, 1.5f));
            epr.setPosition(fx + 7.f, fy + 0.5f); epr.setFillColor(eyePupil); target.draw(epr);
            sf::RectangleShape brl(sf::Vector2f(3.f, 0.8f));
            brl.setPosition(fx - 0.5f, fy - 1.5f); brl.setFillColor(hair); target.draw(brl);
            sf::RectangleShape brr(sf::Vector2f(3.f, 0.8f));
            brr.setPosition(fx + 6.f, fy - 1.5f); brr.setFillColor(hair); target.draw(brr);
            sf::RectangleShape mouth(sf::Vector2f(2.5f, 0.8f));
            mouth.setPosition(fx + 2.5f, fy + 4.f);
            mouth.setFillColor(sf::Color(180, 120, 100));
            target.draw(mouth);
            break;
        }
        case Dir::Left: {
            float fx = px + 14.f;
            sf::RectangleShape seye(sf::Vector2f(2.f, 2.f));
            seye.setPosition(fx, fy + 1.f); seye.setFillColor(eyeWhite); target.draw(seye);
            sf::RectangleShape sep(sf::Vector2f(1.2f, 1.2f));
            sep.setPosition(fx + 0.2f, fy + 1.5f); sep.setFillColor(eyePupil); target.draw(sep);
            break;
        }
        case Dir::Right: {
            float fx = px + 16.f;
            sf::RectangleShape seye2(sf::Vector2f(2.f, 2.f));
            seye2.setPosition(fx, fy + 1.f); seye2.setFillColor(eyeWhite); target.draw(seye2);
            sf::RectangleShape sep2(sf::Vector2f(1.2f, 1.2f));
            sep2.setPosition(fx + 0.6f, fy + 1.5f); sep2.setFillColor(eyePupil); target.draw(sep2);
            break;
        }
        default: break;
        }
    }

    // === 10. 衣领 ===
    if (m_dir == Dir::Down) {
        sf::RectangleShape collar(sf::Vector2f(6.f, 2.f));
        collar.setPosition(px + 13.f, py + 10.f + bodyBob);
        collar.setFillColor(sf::Color(200, 225, 255, 100));
        target.draw(collar);
    }
}

void PlayerSprite::PreRenderCharacter() {
}