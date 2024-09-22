# Project Documentation

## 1. Project Overview

### IntelliShell

### 1.1 Project Statement

- **Problem Statement**: When new programmers begin their learning journey, unix commands often stand in their way. Many of these commands have non-intuitive syntax that is hard to memorize without practice. This stands in the way of effective development as these commands are the foundation of most medium to low-level programming.
- **Objectives**:
  - To allow users to easily find unix commands within the shell by interacting with it through natural language.
  - To enable easier understanding and retention of unix commands long-term.
- **Target Audience**:
  - Anyone that is new to programming, and needs to use the shell.
  - Those who have worked with high-level languages, but want to explore how to effectively use lower-level tools and begin development.
  - Those who have no idea what the shell is, as this solution will provide an approachable medium for anyone to learn.

### 1.2 Proposed Solution

- **Solution Overview**: A unix shell written in C that has all the features of a standard unix shell, but augmented with generative AI to assist the user in learning how to use the commands effectively.
- **Key Features**:

  - User has the ability to use the interactive shell just like they would a normal unix shell, as all the commands will be available
  - When a user enters a command thats incorrect (i.e. results in an error), generative AI will summarize the error message and provide helpful tips to remedy.
  - User can prompt the shell with questions to find the command they're looking for if they're unaware of the exact syntax, and the integrated generative AI will provide them some recommendations.
  - When a user enters a command that does not exist, the shell will automatically provide them some similar commands using generative AI.

- **Expected Impact**:
  - Will create a lower barrier to entry for those learning the unix shell and its many commands.
  - Students who are new to Computer Science will be able learn essential commands quicker, leading to better academic results.
  - Those who have no computer science knowledge will be able to use their computers with greater efficiency using the terminal/shell.

### 1.3 Technical Aspects

- **Programming Languages**: We have built the interactive shell using C because it is easiest for creating with low-level programs such as this custom shell.
- **Frameworks and Libraries**:
  - **libcurl** for handing the HTTP requests, specifically querying the desired generative AI API. In the future, this will not be necessary as we will run generations on-device. However, this addition will limit who can run this tool as powerful computational resources are required to run generative AI locally.
  - **libcjson** for handing the formatted JSON that is returned from the desired generative AI API. The generated content must be separated from the rest of the JSON object so it can be displayed in the shell.
  - **libreadline** for keeping track of previously entered commands in the custom shell. This will allow the user to go back to their previous commands using arrow keys.

## 2. Documentation of AI Tools Used

### 2.1 Overview of AI Tools

- **Tool Name**:
  - CodeWhisperer
  - ChatGPT
  - gpt-4o-mini
- **Purpose**: CodeWhisperer and ChatGPT were used to generate small sections of code and advise on best-practices for the development of this application. gpt-4o-mini is sed to generate the responses to users for the main unix utility.

### 2.2 Application of AI Tools

- **When Applied**: Specify when each tool was applied during the project phases (e.g., brainstorming, developing)
  - That was come.
- **How Applied**: Detail the methods or processes through which the AI tool was utilized.
- **Rationale**: Explain why each tool was selected for its specific application.

### 2.3 Total use of AI Tools

- **Participant 1 - 80%**: Used AI tools brainstorming the best implementation method, and how to debug C on Windows.
- **Participant 2 - 50%**: Mainly used the tools as a fast way to lookup best practices in C and exact syntax when they knew what they needed, but not the exact code.
- **Participant 3 - 0%**: No use of AI tools
