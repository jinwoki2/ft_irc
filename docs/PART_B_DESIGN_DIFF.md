# B파트 설계 비교서: Confluence 초안 vs 최종 설계 (`ARCHITECTURE_AND_SPRINT.md`)

> **작성 목적**: 팀 Confluence에 기록된 초기 B파트 설계와 현재 TDD 기반 최종 설계([docs/ARCHITECTURE_AND_SPRINT.md](ARCHITECTURE_AND_SPRINT.md))의 **공통점(유지된 팀 합의)**, **차이점(구체화·개선 사항)**, 그리고 **통합 시 A파트와 합의가 필요한 경계 변경 사항**을 한눈에 비교한다.
>
> **비교 기준 문서**:
> - **Confluence (초기 팀 합의)**: [클래스 / 데이터 구조 설계서](https://rainstar.atlassian.net/wiki/spaces/I/pages/2523340), [IRC / Command 설계](https://rainstar.atlassian.net/wiki/spaces/I/pages/5242881/IRC+Command), [네트워크 / I-O 처리 설계서](https://rainstar.atlassian.net/wiki/spaces/I/pages/2490602/I-O), [[설계] A/B 파싱 역할 분리](https://rainstar.atlassian.net/wiki/spaces/I/pages/8093841/A+B), [시스템 아키텍처 개요서](https://rainstar.atlassian.net/wiki/spaces/I/pages/2654382)
> - **현재 최종 설계**: [docs/ARCHITECTURE_AND_SPRINT.md](ARCHITECTURE_AND_SPRINT.md)

---

## 1. 한눈에 보는 요약

| 구분 | Confluence B파트 설계 | 현재 최종 설계 (`ARCHITECTURE_AND_SPRINT.md`) | A파트 영향 |
| :--- | :--- | :--- | :---: |
| **상위 조립 구조** | `IrcServer`가 `IrcCore` + `Server` 합성 | `IrcServer`가 `SystemClock` + `IrcProtocol` + `Server` 합성 | 경미 (조립부) |
| **연결 식별자** | **`int fd` 공통 사용** (OS `fd` 재사용) | **`int fd` 공통 사용** (OS `fd` 재사용 동일 유지) | 없음 (동일) |
| **프레이밍 위치** | A가 `\r\n` 분리 후 완성된 한 줄 전달 | B는 완성된 한 줄(`onMessage(fd, line)`) 기준으로 개발, 통합 시 A 유지 vs B 이전 최종 결정 | 통합 시 결정 |
| **A ↔ B 경계 방식** | `Server`가 `IrcCore&` 직접 참조, `IncomingMessage`/`OutgoingMessage` 구조체 교환 | 순수 인터페이스 **`IProtocol`** / **`ISender`** 도입 (`send`, `close`) | **합의 필요** |
| **시간 의존성** | 명시 없음 (`std::time` 직접 사용 암묵 가정) | 순수 인터페이스 **`IClock`** 도입 (`SystemClock` / `StubClock`) | 없음 (B 내부) |
| **중심 객체 구조** | `IrcCore`가 map 3개와 파싱·디스패치·응답 모두 보유 | `IrcProtocol`이 6개 전문 컴포넌트(`MessageParser`, `ClientRegistry`, `ChannelRegistry`, `ReplyBuilder`, `CommandDispatcher`, `IrcCaseMapping`) 합성 | 없음 (B 내부) |
| **Command 구조** | 순수 인터페이스 `Command` + `IrcCore&` + `vector<OutgoingMessage>&` 전달 | 추상 클래스 **`ACommand`**(`requiresRegistration`, `minParams`) + **`CommandContext`** 전달 | 없음 (B 내부) |
| **지원 명령 수** | 9종 (`PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`, `KICK`, `INVITE`, `TOPIC`, `MODE`) | **16종** (실제 클라이언트 호환용 `CAP`, `PING`, `PONG`, `QUIT`, `PART`, `NAMES`, `NOTICE` 추가) | 없음 (B 내부) |
| **MODE 처리** | `ModeCommand` 단일 클래스 | `ModeCommand` + **`IModeHandler` 전략 패턴 5종** (`+i/+t/+k/+o/+l`) + Irssi 호환(`MODE #c b`, 유저 모드) | 없음 (B 내부) |
| **테스트 전략** | 통합 테스트 케이스 위주 | *TDD for Embedded C* 스타일 — `SpySender`/`StubClock`으로 CppUTest 100% 독립 TDD | 없음 (B 내부) |

---

## 2. 그대로 유지된 팀 합의 (공통점)

Confluence 설계서의 핵심 원칙과 A/B 책임 경계는 **그대로 유지**된다.

1. **A/B 책임 분리 원칙**: A는 TCP 소켓·non-blocking I/O·`poll()`만 담당하고 IRC 의미를 해석하지 않으며, B는 소켓 I/O를 직접 수행하지 않고 IRC 상태·명령·응답만 담당한다.
2. **`IrcServer` 상위 합성 구조**: 상속 없이 `IrcServer`가 A와 B를 멤버로 소유해 조립한다.
3. **`int fd` 공통 식별자 & OS `fd` 재사용**: 별도의 증가 ID(`ConnectionId`)나 `_fdToId` 매핑 없이 A의 `std::map<int, Connection>`과 동일한 `int fd`를 B에서도 사용한다.
4. **Disconnect 정리 순서**: A가 연결 종료 감지 → B에 `onDisconnect(fd)` 먼저 호출해 `Client`/채널 정리 → 그다음 A가 `pollFds` 제거 + `::close(fd)` + `Connection` 삭제.
5. **완성된 한 줄 단위 처리**: B의 파서·도메인·명령 계층은 `\r\n`이 제거된 완성된 한 줄(`line`)을 입력으로 받는다.

---

## 3. 항목별 상세 차이점과 변경 이유

### 3-1. A ↔ B 경계 인터페이스 (`IProtocol`, `ISender`, `IClock`)

| 항목 | Confluence 설계 | 현재 최종 설계 |
| :--- | :--- | :--- |
| **A → B 호출** | `IrcCore::onMessage(const IncomingMessage&)` + `onConnect(fd)` / `onDisconnect(fd)` | `IProtocol` 순수 인터페이스:<br>• `setSender(ISender*)`<br>• `onConnect(int fd, const std::string& host)`<br>• `onMessage(int fd, const std::string& line)`<br>• `onDisconnect(int fd)` |
| **B → A 송신** | `Command::execute`가 `std::vector<OutgoingMessage>& output`을 채워서 반환 → A의 `Server::queueMessage()` 호출 | `ISender` 순수 인터페이스:<br>• `send(int fd, const std::string& data)`<br>• `close(int fd)` *(Flush-then-Close 요청)* |
| **시스템 시각** | 미정의 | `IClock` 순수 인터페이스 (`now()`, `nowFormatted()`) |

- **왜 바꿨는가?**
  1. **테스트 격리 (CppUTest TDD)**: B가 `Server` 구체 클래스나 `std::time()`에 직접 의존하지 않도록 `ISender`·`IClock` 순수 인터페이스 뒤에 숨겨, 테스트 시 `SpySender`·`StubClock`을 꽂아 소켓 없이 0ms 단위 테스트를 돌리기 위함이다.
  2. **B 주도 연결 종료(`QUIT`) 지원**: Confluence 초안에는 `OutgoingMessage`로 데이터 전송만 표현되어 있었고, `QUIT` 명령 시 **"B가 `ERROR` 응답을 보낸 뒤 A에게 연결 종료를 요청하는 경로"** 가 빠져 있었다. `ISender::close(int fd)`로 이 계약을 명시했다.
  3. **클라이언트 `host` 전달**: IRC 브로드캐스트 형식(`:nick!user@host ...`)을 정확히 만들려면 접속 시 `onConnect(int fd, const std::string& host)`로 호스트 정보가 필요하다.
  4. **기존 A파트 코드 재사용 가능**: `Server`가 `ISender`를 구현할 때 `send(fd, data)` 안에서 기존 `queueMessage(OutgoingMessage{fd, data})`를 그대로 호출할 수 있어 A파트 수정 폭이 매우 작다.

---

### 3-2. 중심 객체 분해 (`IrcCore` → `IrcProtocol` + 전문 컴포넌트 6종)

Confluence 초안의 `IrcCore`는 다음과 같이 모든 상태를 직접 보유했다:
```cpp
// Confluence 초안
class IrcCore {
    std::map<int, Client>           _clients;
    std::map<std::string, Channel>  _channels;
    std::map<std::string, Command*> _commands;
};
```

현재 최종 설계에서는 **`IrcProtocol`** 이 진입점/조립 역할만 맡고, 내부 책임을 단일 책임 원칙(SRP)에 따라 6개로 분리했다:

1. **`IrcCaseMapping`**: RFC 2812 §2.2 스칸디나비아 대소문자 정규화(`[]\^` ↔ `{}|~`) 및 닉네임(1~9자)/채널명(`#`/`&`, 2~50자)/키(1~23자) 유효성 검증
2. **`MessageParser` (+ `Message`)**: Confluence에서는 `Message`가 데이터 보관과 파싱을 겸했으나, 데이터 모델(`Message`)과 파싱 로직(`MessageParser`: 최대 15개 파라미터, `:trailing`, 대소문자 정규화, 빈 줄/NUL 방어)을 분리
3. **`ClientRegistry`**:
   - `_byFd` (`std::map<int, Client*>`) + `_fdByNormNick` (`std::map<std::string, int>`) **이중 인덱스** 보유
   - Confluence 문서의 *"nickname 중복 검사에 별도 index가 필요한가?"* 질문에 대한 구체적 해답 ($O(\log N)$ 중복 검사 및 조회)
4. **`ChannelRegistry`**: 정규화 채널명 기반 생성/조회, disconnect 시 사용자가 속한 모든 채널에서 일괄 제거 및 빈 채널 자동 삭제
5. **`ReplyBuilder` (+ `NumericCode` enum)**: `:serverName XXX target ... :trailing\r\n`, `001~004` Welcome, 이벤트 브로드캐스트, `ERROR` 포맷 생성 일원화
6. **`CommandDispatcher`**: 명령 조회뿐 아니라 **공통 사전 검증**(`451 ERR_NOTREGISTERED`, `461 ERR_NEEDMOREPARAMS`, `421 ERR_UNKNOWNCOMMAND`)을 각 명령에서 중복 구현하지 않도록 중앙 처리

---

### 3-3. 도메인 모델(`Client`, `Channel`) 세부 확장

| 클래스 | Confluence 초안 | 현재 최종 설계 추가/구체화 사항 |
| :--- | :--- | :--- |
| **`Client`** | `_fd`, `_nickname`, `_username`, `_realname`, `_passwordAccepted`, `_registered` | • **`_host`**: `:nick!user@host` prefix 생성용<br>• **`_hasNick`, `_hasUser`**: 등록 완료 조건(`PASS` + `NICK` + `USER`)의 명확한 상태 머신<br>• **`_capNegotiating`**: Irssi 등 Reference Client가 접속 시 `CAP LS` → `CAP END` 협상을 완료할 때까지 `001~004` Welcome을 보류하기 위한 상태 |
| **`Channel`** | `name`, `members`, `operators`, `invited`, `topic`, `key`, `+i`, `+t`, `+l` | • **`_topicSetter`, `_topicSetAt`**: 토픽 설정자 및 설정 시각(`IClock`) 보관<br>• 멤버/Op/초대 목록을 **`std::set<int>` (`fd` 기준)** 으로 확정<br>• **`canJoin(fd, key, errCode)`**: `+i`/`+k`/`+l` 검증과 해당 RFC 에러 코드 반환을 도메인 메서드로 캡슐화 |

---

### 3-4. `Command` 계층과 명령 목록 확장

- **인터페이스 → 추상 클래스 (`Command` → `ACommand`)**:
  - Confluence는 `execute()`만 있는 순수 인터페이스 `Command`를 제안했으나, 현재 설계는 `requiresRegistration()`(기본 `true`)과 `minParams()`(기본 `0`) 기본 구현을 제공하는 추상 클래스 `ACommand`를 사용해 공통 검사를 `CommandDispatcher`에 위임한다.
- **실행 컨텍스트 (`IrcCore&` + `vector<OutgoingMessage>&` → `CommandContext&`)**:
  - 각 Command가 `IrcProtocol` 전체 내부에 결합되는 대신, `CommandContext`를 통해 `clients`, `channels`, `replies`, `sender`, `clock`과 브로드캐스트 헬퍼(`sendNumeric`, `broadcastChannel`, `broadcastPeers`)만 참조한다.
- **지원 명령 9종 → 16종 확장**:
  - Confluence 기본 9종: `PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`, `KICK`, `INVITE`, `TOPIC`, `MODE`
  - **추가 7종 (Reference Client 및 RFC 필수 흐름 대응)**:
    - `CAP`: Irssi/HexChat/WeeChat 초기 접속 협상 대응
    - `PING` / `PONG`: 연결 유지(keep-alive) 및 클라이언트 응답 처리
    - `QUIT`: 정상 종료 시 피어 브로드캐스트 + `ERROR` + `close(fd)` + 즉시 정리
    - `PART`: 채널 탈퇴 브로드캐스트 및 빈 채널 삭제
    - `NAMES`: 채널 멤버 목록(`353`/`366`) 조회
    - `NOTICE`: `PRIVMSG`와 동일 라우팅이되 RFC 2812 §3.3.2에 따라 에러 응답 절대 미생성
- **`MODE` 전략 패턴 (`IModeHandler` 5종)**:
  - `ModeCommand` 내부 분기 폭증을 막기 위해 `i`/`t`/`k`/`o`/`l` 핸들러를 `IModeHandler` 구현체 5종으로 분리했다.
  - 또한 Irssi가 채널 입장/접속 직후 자동으로 보내는 `MODE #chan b` (`368 RPL_ENDOFBANLIST`) 및 `MODE <nick> +i` (`221 RPL_UMODEIS`) 호환 처리를 설계에 포함했다.

---

### 3-5. 프레이밍(`StreamFramer`)에 대한 입장 차이

- **Confluence ([[설계] A/B 파싱 역할 분리](https://rainstar.atlassian.net/wiki/spaces/I/pages/8093841/A+B))**: A파트가 `\r\n` 프레이밍(TCP Stream Framing)을 수행하고, B파트는 완성된 한 줄을 파싱한다. (실제 `src/Server.cpp`에 이미 `IRC-33`으로 구현됨)
- **현재 최종 설계**: B파트는 우선 **Confluence 합의와 동일하게 완성된 한 줄을 받는 `onMessage(int fd, const std::string& line)`을 중심 진입점**으로 구현한다. 단, 통합 단계(S3-1)에서 프로토콜 계층 원칙에 따라 프레이밍을 B로 옮기기로 합의하면 `onMessage` 앞단에 `StreamFramer` 어댑터(`onReceive`) 1개만 추가할 수 있도록 열어 두었다(기존 B 코드 수정 불필요).

---

## 4. 통합 단계(S3-1)에서 A파트와 합의가 필요한 체크리스트

Confluence [클래스 / 데이터 구조 설계서](https://rainstar.atlassian.net/wiki/spaces/I/pages/2523340)의 경계 변경 원칙에 따라, **B파트 내부 구조 변경(3-2 ~ 3-4)은 A파트에 영향을 주지 않는다.**  
아래 **4가지 A ↔ B 경계 항목만** 통합 전 A파트 담당자와 최종 확인하면 된다.

1. **`Server`의 `ISender` 구현 추가**
   - `send(int fd, const std::string& data)`: 기존 `queueMessage(OutgoingMessage{fd, data})`에 위임 가능
   - `close(int fd)`: `Connection::_closingAfterFlush = true`로 표시하고, `handleWrite()`에서 `_sendBuffer`가 모두 전송된 뒤 소켓을 닫는 **Flush-then-Close** 추가
2. **`Server`의 `IProtocol` 이벤트 3종 호출 연결**
   - `acceptConnection()`: 새 연결 수락 시 `_protocol.onConnect(clientFd, host)` 호출
   - `handleRead()`: 완성된 줄마다 `_protocol.onMessage(fd, line)` 호출 (기존 `_core.onMessage(IncomingMessage)` 대체 또는 어댑터 유지)
   - `disconnectClient()`: **`_protocol.onDisconnect(fd)`를 `::close(fd)`보다 먼저 동기 호출** (OS `fd` 재사용 안전 보장)
3. **`handleRead()` 루프의 조기 중단 안전장치**
   - 한 패킷에 `QUIT\r\nPRIVMSG ...\r\n`이 같이 들어왔을 때, `QUIT` 처리 중 `close(fd)`가 호출되어 `_closingAfterFlush == true`가 되면 `handleRead()`는 즉시 루프를 빠져나가야 한다 (댕글링 참조 `recvBuffer` 및 `QUIT` 이후 명령 실행 방지).
4. **프레이밍 최종 위치 및 개행/NUL 정책 합의**
   - Option A (A 프레이밍 유지): `Server::handleRead()`에 `nc` 단독 `\n` 호환 및 NUL(`0x00`) 방어 보완 여부 결정
   - Option B (B 프레이밍 이전): A는 raw bytes만 넘기고 B 앞단에 `StreamFramer` 추가 여부 결정
