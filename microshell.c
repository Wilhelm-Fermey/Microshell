/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wfermey <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/14 11:46:34 by wfermey           #+#    #+#             */
/*   Updated: 2022/06/14 13:59:18 by wfermey          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static int ft_strlen(char *str)
{
	int i = 0;
	while (str[i])
		i++;
	return i;
}

static void error(char *msg, char *arg)
{
	write(STDERR_FILENO, msg, ft_strlen(msg));
	if (arg)
		write(STDERR_FILENO, arg, ft_strlen(arg));
	write(STDERR_FILENO, "\n", 1);
	exit(0);
}

static void ft_cd(char **argv)
{
	if (!argv[1] || argv[2])
		error("cd: bad arguments", NULL);
	if (chdir(argv[1]) == -1)
		error("cd: could not change directory to: ", argv[1]);
	exit(0);
}


static int ft_exec(char **argv, int fdd, int *fd, char **env)
{
	int i = 0;
	int last = 0;
	pid_t pid;

	while (argv[i] && strcmp(argv[i], "|") != 0)		// si argv[i] != '|' -> i++;
		i++;
	if (!argv[i])	
		last = 1;
	argv[i] = NULL;									// argv[i] == NULL. On garde que le 1er arg.
	if (pipe(fd) == -1)							// on pipe end.
		error("error: fatal1", NULL);
	pid = fork();									// on fork le program.
	if (pid == -1)
		error("error: fatal2", NULL);
	if (pid == 0)									// si pid enfant.
	{
		if (dup2(fdd, 0) == -1)						// on lis dans fdd a la place de fd 0;
			error("error: fatal3", NULL);
		if (last == 0)								// si pas dernier.
		{
			if (dup2(fd[1], 1) == -1)	// on ecris dans pipe a la place de 1;
				error("error: fatal4", NULL);
		}
		close (fdd);
		close(fd[0]);								// on close;
		close(fd[1]);
		if (strcmp(argv[0], "cd") == 0)				// on appel cd si besoin.
			ft_cd(argv);
		if (execve(argv[0], argv, env) == -1)			// on execve.
			error("error: cannot execute ", argv[0]);
	}
	else											// si pid parent.
	{
		if (dup2(fd[0], fdd) == -1)					// fdd devient fd[0]
			error("error: fatal", NULL);
		close(fd[0]);
		close(fd[1]);
		waitpid(pid, NULL, 0);
	}
	if (last == 0)
		return i + 1;
	return i;
}

static void ft_cmd(char ** argv, char **env)
{
	int fdd;
	int i = 0;
	int fd[2];

	fdd = dup(0);
	if (!argv[0])
		return;
	while (argv[i])
		i += ft_exec(argv + i, fdd, fd, env);
	close(fdd);
	close(fd[1]);
	close(fd[0]);
}

int main(int argc, char **argv, char **env)
{
	if (argc < 2)
		return (0);
	int i = 1;
	int start = 1;
	while (argv[i])
	{
		if (strcmp(argv[i], ";") == 0)
		{
			argv[i] = NULL;
			ft_cmd(argv + start, env);
			i++;
			while (argv[i] && strcmp(argv[i], ";") == 0)
				i++;
			start = i;
		}
		else
			i++;
	}
	ft_cmd(argv + start, env);
}

